#include "hepch.h"
#include "vulkan_buffer.h"

// Internal
#include "vulkan_instance.h"
#include "vulkan_device.h"
#include "vulkan_command_buffer.h"
#include "vulkan_descriptor.h"

namespace hellengine
{

	namespace graphics
	{

		VulkanBuffer::VulkanBuffer()
		{
			m_handle = VK_NULL_HANDLE;
			m_memory = VK_NULL_HANDLE;
			m_size = 0;
			m_type = BufferType_None;
		}

		VulkanBuffer::~VulkanBuffer()
		{
			NO_OP;
		}

		void VulkanBuffer::Create(const VulkanInstance& instance, const VulkanDevice& device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, b8 shared, u32 queue_count, u32* queue_indices)
		{
			VkBufferCreateInfo buffer_info = {};
			buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			buffer_info.pNext = VK_NULL_HANDLE;
			buffer_info.flags = 0;
			buffer_info.size = size;
			buffer_info.usage = usage;
			buffer_info.sharingMode = shared ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE;
			buffer_info.queueFamilyIndexCount = queue_count;
			buffer_info.pQueueFamilyIndices = queue_indices;

			VK_CHECK(vkCreateBuffer(device.GetLogicalDevice(), &buffer_info, instance.GetAllocator(), &m_handle));

			VkMemoryRequirements mem_requirements;
			vkGetBufferMemoryRequirements(device.GetLogicalDevice(), m_handle, &mem_requirements);

			u32 memory_type_index = U32MAX;
			memory_type_index = VulkanDevice::FindMemoryType(device, mem_requirements.memoryTypeBits, properties);

			VkMemoryAllocateInfo alloc_info = {};
			alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			alloc_info.pNext = VK_NULL_HANDLE;
			alloc_info.allocationSize = mem_requirements.size;
			alloc_info.memoryTypeIndex = memory_type_index;

			VK_CHECK(vkAllocateMemory(device.GetLogicalDevice(), &alloc_info, instance.GetAllocator(), &m_memory));
			VK_CHECK(vkBindBufferMemory(device.GetLogicalDevice(), m_handle, m_memory, 0));

			m_size = size;
		}

		void VulkanBuffer::Destroy(const VulkanInstance& instance, const VulkanDevice& device) const
		{
			vkDestroyBuffer(device.GetLogicalDevice(), m_handle, instance.GetAllocator());
			vkFreeMemory(device.GetLogicalDevice(), m_memory, instance.GetAllocator());
		}

		void VulkanBuffer::MapUnmap(const VulkanDevice& device, VkDeviceSize size, VkDeviceSize offset, void* data) const
		{
			void* data_ptr;
			VK_CHECK(vkMapMemory(device.GetLogicalDevice(), m_memory, offset, size, 0, &data_ptr));
			memcpy(data_ptr, data, size);
			vkUnmapMemory(device.GetLogicalDevice(), m_memory);
		}

		void VulkanBuffer::Map(const VulkanDevice& device, VkDeviceSize size, VkDeviceSize offset, void*& data) const
		{
			VK_CHECK(vkMapMemory(device.GetLogicalDevice(), m_memory, offset, size, 0, &data));
		}

		void VulkanBuffer::Unmap(const VulkanDevice& device) const
		{
			vkUnmapMemory(device.GetLogicalDevice(), m_memory);
		}

		void VulkanBuffer::CopyFromBuffer(const VulkanDevice& device, const VulkanCommandPool& command_pool, VkBuffer src_buffer) const
		{
			VulkanCommandBuffer command_buffer;
			command_buffer.Allocate(device, command_pool, VK_COMMAND_BUFFER_LEVEL_PRIMARY);

			command_buffer.BeginRecordingOneTime(device, command_pool);

			VkBufferCopy copy_region = {};
			copy_region.srcOffset = 0;
			copy_region.dstOffset = 0;
			copy_region.size = m_size;

			vkCmdCopyBuffer(command_buffer.GetHandle(), src_buffer, m_handle, 1, &copy_region);

			command_buffer.EndRecordingOneTime(device, command_pool);

			command_buffer.Free(device, command_pool);
		}

		void VulkanBuffer::CopyToImage(const VulkanDevice& device, const VulkanCommandPool& command_pool, VkImage src_image, u32 width, u32 height) const
		{
			VkBufferImageCopy region = {};
			region.bufferOffset = 0;
			region.bufferRowLength = 0;
			region.bufferImageHeight = 0;

			region.imageSubresource = {
				VK_IMAGE_ASPECT_COLOR_BIT,
				0,
				0,
				1
			};

			region.imageOffset = { 0, 0, 0 };
			region.imageExtent = {
				width,
				height,
				1
			};

			VulkanCommandBuffer command_buffer;
			command_buffer.Allocate(device, command_pool, VK_COMMAND_BUFFER_LEVEL_PRIMARY);
			command_buffer.BeginRecordingOneTime(device, command_pool);

			vkCmdCopyBufferToImage(command_buffer.GetHandle(), m_handle, src_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

			command_buffer.EndRecordingOneTime(device, command_pool);
			command_buffer.Free(device, command_pool);
		}

		VulkanUniformBuffer::VulkanUniformBuffer()
		{
			m_handle = VK_NULL_HANDLE;
			m_memory = VK_NULL_HANDLE;
			m_mapped_memory = VK_NULL_HANDLE;
			m_size = 0;
			m_dynamic_alignment = 0;
			m_type = BufferType_None;
		}

		VulkanUniformBuffer::~VulkanUniformBuffer()
		{
			NO_OP;
		}

} // namespace graphics

} // namespace hellengine