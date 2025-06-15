#pragma once

// Internal
#include "shared.h"

namespace hellengine
{

	namespace graphics
	{

		class VulkanBuffer
		{
		public:
			VulkanBuffer();
			~VulkanBuffer();

			void Create(const VulkanInstance& instance, const VulkanDevice& device, VkDeviceSize size, VkBufferUsageFlags usage, b8 shared, u32 queue_count, u32* queue_indices, VkMemoryPropertyFlags properties);
			void Destroy(const VulkanInstance& instance, const VulkanDevice& device) const;

			void MapUnmap(const VulkanDevice& device, VkDeviceSize size, VkDeviceSize offset, const void* data) const;
			void Map(const VulkanDevice& device, VkDeviceSize size, VkDeviceSize offset, void*& data) const;
			void Unmap(const VulkanDevice& device) const;

			VkBuffer GetHandle() const { return m_handle; }
			VkDeviceSize GetSize() const { return m_size; }
			VkBufferUsageFlags GetUsage() const { return m_usage; }

			VkDeviceMemory GetDeviceMemory() const { return m_memory; }
			VkMemoryPropertyFlags GetMemoryFlags() const { return m_properties; }

			u32 GetCount() const { return (u32)m_size / sizeof(u32); }

			void SetType(BufferType type) { m_type = type; }
			BufferType GetType() const { return m_type; }

			void CopyFromBuffer(const VulkanDevice& device, const VulkanCommandPool& command_pool, VkBuffer src_buffer, u32 src_size = 0, u32 offset = 0) const;
			void CopyToImage(const VulkanDevice& device, const VulkanCommandPool& command_pool, VkImage src_image, std::vector<VkBufferImageCopy> buffer_ranges) const;
			void CopyFromImage(const VulkanDevice& device, const VulkanCommandPool& command_pool, VkImage src_image, std::vector<VkBufferImageCopy> buffer_ranges) const;
			
		protected:
			VkBuffer m_handle;
			VkDeviceSize m_size;
			VkBufferUsageFlags m_usage;

			VkDeviceMemory m_memory;
			VkMemoryPropertyFlags m_properties;

			BufferType m_type;
		};

		class VulkanMappedBuffer : public VulkanBuffer
		{
		public:
			VulkanMappedBuffer();
			~VulkanMappedBuffer();

			void CreateMapped(const VulkanInstance& instance, const VulkanDevice& device, VkDeviceSize elem_size, u32 elem_count, VkBufferUsageFlags usage, b8 shared, u32 queue_count, u32* queue_indices, VkMemoryPropertyFlags properties, b8 persistent);

			u32 GetStride() const { return m_stride; }
			void*& GetMappedMemory() { return m_mapped_memory; }

		protected:
			void* m_mapped_memory;
			u32 m_stride;
		};

		class VulkanUniformBuffer : public VulkanMappedBuffer
		{
		public:
			VulkanUniformBuffer();
			~VulkanUniformBuffer();
		};

		class VulkanStorageBuffer : public VulkanMappedBuffer
		{
		public:
			VulkanStorageBuffer();
			~VulkanStorageBuffer();
		};

	} // namespace graphics

} // namespace hellengine