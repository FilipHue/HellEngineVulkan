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

			void Create(const VulkanInstance& instance, const VulkanDevice& device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, b8 shared = false, u32 queue_count = 0, u32* queue_indices = VK_NULL_HANDLE);
			void Destroy(const VulkanInstance& instance, const VulkanDevice& device) const;

			void MapUnmap(const VulkanDevice& device, VkDeviceSize size, VkDeviceSize offset, const void* data) const;
			void Map(const VulkanDevice& device, VkDeviceSize size, VkDeviceSize offset, void*& data) const;
			void Unmap(const VulkanDevice& device) const;

			VkBuffer GetHandle() const { return m_handle; }
			VkDeviceMemory GetDeviceMemory() const { return m_memory; }
			VkDeviceSize GetSize() const { return m_size; }
			u32 GetCount() const { return (u32)m_size / sizeof(u32); }

			void SetType(BufferType type) { m_type = type; }
			BufferType GetType() const { return m_type; }

			void CopyFromBuffer(const VulkanDevice& device, const VulkanCommandPool& command_pool, VkBuffer src_buffer, u32 src_size = 0, u32 offset = 0) const;
			void CopyToImage(const VulkanDevice& device, const VulkanCommandPool& command_pool, VkImage src_image, std::vector<VkBufferImageCopy> buffer_ranges) const;
			
		protected:
			VkBuffer m_handle;
			VkDeviceMemory m_memory;
			VkDeviceSize m_size;

			BufferType m_type;
		};

		class VulkanUniformBuffer : public VulkanBuffer
		{
		public:
			VulkanUniformBuffer();
			~VulkanUniformBuffer();

			void SetDynamicAlignment(u32 dynamic_alignment) { m_dynamic_alignment = dynamic_alignment; }
			u32 GetDynamicAlignment() const { return m_dynamic_alignment; }

			void*& GetMappedMemory() { return m_mapped_memory; }

		private:
			u32 m_dynamic_alignment;
			void* m_mapped_memory;
		};

		class VulkanStorageBuffer : public VulkanBuffer
		{
		public:
			VulkanStorageBuffer();
			~VulkanStorageBuffer();

			void SetDynamicAlignment(u32 dynamic_alignment) { m_dynamic_alignment = dynamic_alignment; }
			u32 GetDynamicAlignment() const { return m_dynamic_alignment; }

			void*& GetMappedMemory() { return m_mapped_memory; }

		private:
			u32 m_dynamic_alignment;
			void* m_mapped_memory;
		};

	} // namespace graphics

} // namespace hellengine