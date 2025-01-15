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

			m_width = 0;
			m_height = 0;
			m_depth = 0;

			m_image_view = VK_NULL_HANDLE;
		}

		VulkanImage::~VulkanImage()
		{
			NO_OP;
		}

		void VulkanImage::Create(const VulkanInstance& instance, VulkanDevice& device, VkImageType image_type, VkFormat format, u32 width, u32 height, u32 depth, VkImageUsageFlags usage, VkImageTiling tiling, VkMemoryPropertyFlags properties)
		{
			m_width = width;
			m_height = height;
			m_depth = depth;

			VkImageCreateInfo image_info = {};
			image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			image_info.imageType = image_type;
			image_info.extent.width = width;
			image_info.extent.height = height;
			image_info.extent.depth = depth;
			image_info.mipLevels = 1;
			image_info.arrayLayers = 1;
			image_info.format = format;
			image_info.tiling = tiling;
			image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			image_info.usage = usage;
			image_info.samples = VK_SAMPLE_COUNT_1_BIT;
			image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

			VK_CHECK(vkCreateImage(device.GetLogicalDevice(), &image_info, nullptr, &m_handle));

			VkMemoryRequirements memory_requirements;
			vkGetImageMemoryRequirements(device.GetLogicalDevice(), m_handle, &memory_requirements);	

			VkMemoryAllocateInfo alloc_info = {};
			alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			alloc_info.allocationSize = memory_requirements.size;
			alloc_info.memoryTypeIndex = VulkanDevice::FindMemoryType(device, memory_requirements.memoryTypeBits, properties);

			HE_ASSERT(alloc_info.memoryTypeIndex != -1, "Failed to find suitable memory type!");

			VK_CHECK(vkAllocateMemory(device.GetLogicalDevice(), &alloc_info, instance.GetAllocator(), &m_memory));
			VK_CHECK(vkBindImageMemory(device.GetLogicalDevice(), m_handle, m_memory, 0));
		}

		void VulkanImage::Destroy(const VulkanInstance& instance, const VulkanDevice& device) const
		{
			vkDestroyImage(device.GetLogicalDevice(), m_handle, instance.GetAllocator());
			vkFreeMemory(device.GetLogicalDevice(), m_memory, instance.GetAllocator());

			if (m_image_view != VK_NULL_HANDLE)
			{
				DestroyImageView(instance, device, m_image_view);
			}
		}

		void VulkanImage::Transition(const VulkanDevice& device, const VulkanCommandPool& command_pool, VkImage image, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout)
		{
			VkImageMemoryBarrier barrier = {};
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.oldLayout = old_layout;
			barrier.newLayout = new_layout;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.image = image;
			barrier.subresourceRange = {
				VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1
			};

			if (new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
				barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

				if (format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT) {
					barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
				}
			}

			VkPipelineStageFlags source_stage{};
			VkPipelineStageFlags destination_stage{};

			if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
				barrier.srcAccessMask = 0;
				barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

				source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
				destination_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			}
			else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
				barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

				source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
				destination_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			}
			else if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
				barrier.srcAccessMask = 0;
				barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

				source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
				destination_stage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			}
			else {
				HE_GRAPHICS_WARN("Unsupported layout transition!");
			}

			VulkanCommandBuffer command_buffer;
			command_buffer.Allocate(device, command_pool, VK_COMMAND_BUFFER_LEVEL_PRIMARY);
			command_buffer.BeginRecordingOneTime(device, command_pool);

			vkCmdPipelineBarrier(command_buffer.GetHandle(), source_stage, destination_stage, 0, 0, nullptr, 0, nullptr, 1, &barrier);

			command_buffer.EndRecordingOneTime(device, command_pool);
			command_buffer.Free(device, command_pool);
		}

		void CreateImageView(const VulkanInstance& instance, const VulkanDevice& device, VkImage image, VkFormat format, VkImageAspectFlags aspect_flags, VkImageView& image_view)
		{
			VkImageViewCreateInfo create_info = {};
			create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			create_info.flags = 0;
			create_info.pNext = VK_NULL_HANDLE;
			create_info.image = image;
			create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
			create_info.format = format;
			create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
			create_info.subresourceRange.aspectMask = aspect_flags;
			create_info.subresourceRange.baseMipLevel = 0;
			create_info.subresourceRange.levelCount = 1;
			create_info.subresourceRange.baseArrayLayer = 0;
			create_info.subresourceRange.layerCount = 1;

			VK_CHECK(vkCreateImageView(device.GetLogicalDevice(), &create_info, instance.GetAllocator(), &image_view));
		}

		void DestroyImageView(const VulkanInstance& instance, const VulkanDevice& device, VkImageView image_view)
		{
			vkDestroyImageView(device.GetLogicalDevice(), image_view, instance.GetAllocator());
		}

	} // namespace graphics

} // namespace hellengine