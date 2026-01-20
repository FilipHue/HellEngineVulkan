#include "hepch.h"
#include "vulkan_buffer.h"

// Internal
#include "vulkan_instance.h"
#include "vulkan_device.h"
#include "vulkan_command_buffer.h"

namespace hellengine
{

	namespace graphics
	{

		VulkanBuffer::VulkanBuffer()
		{
			m_handle = VK_NULL_HANDLE;
			m_usage = 0;
			m_size = 0;

			m_memory = VK_NULL_HANDLE;
			m_properties = 0;

			m_type = BufferType_None;
		}

		VulkanBuffer::~VulkanBuffer()
		{
			NO_OP;
		}

		void VulkanBuffer::Create(const VulkanInstance& instance, const VulkanDevice& device, VkDeviceSize size, VkBufferUsageFlags usage, b8 shared, u32 queue_count, u32* queue_indices, VkMemoryPropertyFlags properties)
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
			m_usage = usage;
			m_properties = properties;
		}

		void VulkanBuffer::Destroy(const VulkanInstance& instance, const VulkanDevice& device) const
		{
			vkDestroyBuffer(device.GetLogicalDevice(), m_handle, instance.GetAllocator());
			vkFreeMemory(device.GetLogicalDevice(), m_memory, instance.GetAllocator());
		}

		void VulkanBuffer::MapUnmap(const VulkanDevice& device, VkDeviceSize size, VkDeviceSize offset, const void* data) const
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

		void VulkanBuffer::CopyFromBuffer(const VulkanDevice& device, const VulkanCommandPool& command_pool, VkBuffer src_buffer, u32 src_size, u32 offset) const
		{
			VulkanCommandBuffer command_buffer;
			command_buffer.Allocate(device, command_pool, VK_COMMAND_BUFFER_LEVEL_PRIMARY);

			command_buffer.BeginRecordingOneTime(device, command_pool);

			VkBufferCopy copy_region = {};
			copy_region.srcOffset = 0;
			copy_region.dstOffset = offset;
			copy_region.size = src_size == 0 ? m_size : MIN(src_size, m_size);

			vkCmdCopyBuffer(command_buffer.GetHandle(), src_buffer, m_handle, 1, &copy_region);

			command_buffer.EndRecordingOneTime(device, command_pool);

			command_buffer.Free(device, command_pool);
		}

		void VulkanBuffer::CopyToImage(const VulkanDevice& device, const VulkanCommandPool& command_pool, VkImage src_image, std::vector<VkBufferImageCopy> buffer_ranges) const
		{
			VulkanCommandBuffer command_buffer;
			command_buffer.Allocate(device, command_pool, VK_COMMAND_BUFFER_LEVEL_PRIMARY);
			command_buffer.BeginRecordingOneTime(device, command_pool);

			vkCmdCopyBufferToImage(command_buffer.GetHandle(), m_handle, src_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, static_cast<u32>(buffer_ranges.size()), buffer_ranges.data());

			command_buffer.EndRecordingOneTime(device, command_pool);
			command_buffer.Free(device, command_pool);
		}

		void VulkanBuffer::CopyFromImage(const VulkanDevice& device, const VulkanCommandPool& command_pool, VkImage src_image, std::vector<VkBufferImageCopy> buffer_ranges) const
		{
			VulkanCommandBuffer command_buffer;
			command_buffer.Allocate(device, command_pool, VK_COMMAND_BUFFER_LEVEL_PRIMARY);
			command_buffer.BeginRecordingOneTime(device, command_pool);

			vkCmdCopyImageToBuffer(command_buffer.GetHandle(), src_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, m_handle, static_cast<u32>(buffer_ranges.size()), buffer_ranges.data());

			command_buffer.EndRecordingOneTime(device, command_pool);
			command_buffer.Free(device, command_pool);
		}

		VulkanMappedBuffer::VulkanMappedBuffer()
		{
			NO_OP;
		}

		VulkanMappedBuffer::~VulkanMappedBuffer()
		{
			NO_OP;
		}

		void VulkanMappedBuffer::CreateMapped(const VulkanInstance& instance, const VulkanDevice& device, VkDeviceSize elem_size, u32 elem_count, VkBufferUsageFlags usage, b8 shared, u32 queue_count, u32* queue_indices, VkMemoryPropertyFlags properties, b8 persistent)
		{
			m_stride = (u32)ALIGN(elem_size, device.GetProperties().limits.minUniformBufferOffsetAlignment);
			VkDeviceSize size = m_stride * elem_count;
			Create(instance, device, size, usage, shared, queue_count, queue_indices, properties);

			if (persistent && (GetMemoryFlags() & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT))
			{
				Map(device, size, 0, m_mapped_memory);
			}
		}

		VulkanUniformBuffer::VulkanUniformBuffer()
		{
			m_type = BufferType_Uniform;
		}

		VulkanUniformBuffer::~VulkanUniformBuffer()
		{
			NO_OP;
		}

		VulkanStorageBuffer::VulkanStorageBuffer()
		{
			m_type = BufferType_Storage;
		}

		VulkanStorageBuffer::~VulkanStorageBuffer()
		{
			NO_OP;
		}

	} // namespace graphics

} // namespace hellengine