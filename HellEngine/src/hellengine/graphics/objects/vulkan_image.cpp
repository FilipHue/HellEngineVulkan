#include "hepch.h"
#include "vulkan_image.h"

// Internal
#include "vulkan_instance.h"
#include "vulkan_device.h"
#include "vulkan_command_buffer.h"

namespace hellengine
{

	namespace graphics
	{

		VulkanImage::VulkanImage()
		{
			m_handle = VK_NULL_HANDLE;
			m_memory = VK_NULL_HANDLE;
			m_image_view = VK_NULL_HANDLE;

			m_width = 0;
			m_height = 0;
			m_depth = 0;
		}

		VulkanImage::~VulkanImage()
		{
			NO_OP;
		}

		void VulkanImage::Create(const VulkanInstance& instance, const VulkanDevice& device, VkImageType image_type, VkFormat format, VkExtent3D extent, u32 mip_levels, u32 array_layers, VkSampleCountFlagBits sample_count, VkImageTiling tiling, VkImageUsageFlags usage, VkImageCreateFlags flags, VkMemoryPropertyFlags properties)
		{
			m_width = extent.width;
			m_height = extent.height;
			m_depth = extent.depth;

			VkImageCreateInfo image_info = {};
			image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			image_info.pNext = VK_NULL_HANDLE;
			image_info.flags = flags;
			image_info.imageType = image_type;
			image_info.format = format;
			image_info.extent = extent;
			image_info.mipLevels = mip_levels;
			image_info.arrayLayers = array_layers;
			image_info.samples = sample_count;
			image_info.tiling = tiling;
			image_info.usage = usage;
			image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			image_info.queueFamilyIndexCount = 0;
			image_info.pQueueFamilyIndices = nullptr;
			image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

			VK_CHECK(vkCreateImage(device.GetLogicalDevice(), &image_info, nullptr, &m_handle));

			VkMemoryRequirements memory_requirements;
			vkGetImageMemoryRequirements(device.GetLogicalDevice(), m_handle, &memory_requirements);	

			VkMemoryAllocateInfo alloc_info = {};
			alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			alloc_info.pNext = VK_NULL_HANDLE;
			alloc_info.allocationSize = memory_requirements.size;
			alloc_info.memoryTypeIndex = VulkanDevice::FindMemoryType(device, memory_requirements.memoryTypeBits, properties);

			VK_CHECK(vkAllocateMemory(device.GetLogicalDevice(), &alloc_info, instance.GetAllocator(), &m_memory));
			VK_CHECK(vkBindImageMemory(device.GetLogicalDevice(), m_handle, m_memory, 0));
		}

		void VulkanImage::CreateImageView(const VulkanInstance& instance, const VulkanDevice& device, VkImageViewType view_type, VkFormat format, VkImageSubresourceRange range)
		{
			VkImageViewCreateInfo create_info = {};
			create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			create_info.pNext = VK_NULL_HANDLE;
			create_info.flags = 0;
			create_info.image = m_handle;
			create_info.viewType = view_type;
			create_info.format = format;
			create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
			create_info.subresourceRange = range;

			VK_CHECK(vkCreateImageView(device.GetLogicalDevice(), &create_info, instance.GetAllocator(), &m_image_view));
		}

		void VulkanImage::Destroy(const VulkanInstance& instance, const VulkanDevice& device) const
		{
			vkDestroyImage(device.GetLogicalDevice(), m_handle, instance.GetAllocator());
			vkFreeMemory(device.GetLogicalDevice(), m_memory, instance.GetAllocator());
			if (m_image_view != VK_NULL_HANDLE)
			{
				vkDestroyImageView(device.GetLogicalDevice(), m_image_view, instance.GetAllocator());
			}
		}

		void VulkanImage::Transition(const VulkanDevice& device, const VulkanCommandPool& command_pool, VkImage image, VkImageSubresourceRange subresource, VkImageLayout old_layout, VkImageLayout new_layout)
		{
			VkImageMemoryBarrier barrier = {};
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.pNext = VK_NULL_HANDLE;
			barrier.srcAccessMask = GetAccessMask(old_layout);
			barrier.dstAccessMask = GetAccessMask(new_layout);
			barrier.oldLayout = old_layout;
			barrier.newLayout = new_layout;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.image = image;
			barrier.subresourceRange = subresource;

			VulkanCommandBuffer command_buffer;
			command_buffer.Allocate(device, command_pool, VK_COMMAND_BUFFER_LEVEL_PRIMARY);
			command_buffer.BeginRecordingOneTime(device, command_pool);

			vkCmdPipelineBarrier(command_buffer.GetHandle(), GetPipelineStageFlags(old_layout), GetPipelineStageFlags(new_layout), 0, 0, nullptr, 0, nullptr, 1, &barrier);

			command_buffer.EndRecordingOneTime(device, command_pool);
			command_buffer.Free(device, command_pool);
		}

		void VulkanImage::PrepareTransition(const VulkanCommandBuffer& command_buffer, VkImage image, VkImageSubresourceRange subresource, VkImageLayout old_layout, VkImageLayout new_layout)
		{
			VkImageMemoryBarrier barrier = {};
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.pNext = VK_NULL_HANDLE;
			barrier.srcAccessMask = GetAccessMask(old_layout);
			barrier.dstAccessMask = GetAccessMask(new_layout);
			barrier.oldLayout = old_layout;
			barrier.newLayout = new_layout;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.image = image;
			barrier.subresourceRange = subresource;

			vkCmdPipelineBarrier(command_buffer.GetHandle(), GetPipelineStageFlags(old_layout), GetPipelineStageFlags(new_layout), 0, 0, nullptr, 0, nullptr, 1, &barrier);
		}

		void CreateImageView(const VulkanInstance& instance, const VulkanDevice& device, VkImage image, VkImageViewType type, VkFormat format, VkImageSubresourceRange range, VkImageView& image_view)
		{
			VkImageViewCreateInfo create_info = {};
			create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			create_info.pNext = VK_NULL_HANDLE;
			create_info.flags = 0;
			create_info.image = image;
			create_info.viewType = type;
			create_info.format = format;
			create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
			create_info.subresourceRange = range;

			VK_CHECK(vkCreateImageView(device.GetLogicalDevice(), &create_info, instance.GetAllocator(), &image_view));
		}

		void DestroyImageView(const VulkanInstance& instance, const VulkanDevice& device, VkImageView& image_view)
		{
			vkDestroyImageView(device.GetLogicalDevice(), image_view, instance.GetAllocator());
		}

		VkAccessFlags GetAccessMask(VkImageLayout layout)
		{
			switch (layout)
			{
			case VK_IMAGE_LAYOUT_UNDEFINED:
				return 0;
			case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
			case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
				return VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
				return VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
				return VK_ACCESS_SHADER_READ_BIT;
			case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
				return VK_ACCESS_TRANSFER_READ_BIT;
			case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
				return VK_ACCESS_TRANSFER_WRITE_BIT;
			default:
				HE_ASSERT(false, "Unsupported image layout for access mask!");
				return 0;
			}
		}

		VkPipelineStageFlags GetPipelineStageFlags(VkImageLayout layout)
		{
			switch (layout)
			{
			case VK_IMAGE_LAYOUT_UNDEFINED:
				return VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
				return VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
				return VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
				return VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
				return VK_PIPELINE_STAGE_TRANSFER_BIT;
			case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
				return VK_PIPELINE_STAGE_TRANSFER_BIT;
			case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
				return VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			default:
				HE_ASSERT(false, "Unsupported image layout for pipeline stage!");
				return 0;
			}
		}

	} // namespace graphics

} // namespace hellengine