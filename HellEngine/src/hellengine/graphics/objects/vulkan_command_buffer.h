#pragma once

// Internal
#include "shared.h"

namespace hellengine
{

	namespace graphics
	{

		class VulkanCommandPool
		{
		public:
			VulkanCommandPool();
			~VulkanCommandPool();

			void Create(const VulkanInstance& instance, const VulkanDevice& device, u32 queue_family_index);
			void Destroy(const VulkanInstance& instance, const VulkanDevice& device) const;

			VkCommandPool GetHandle() const { return m_handle; }
			u32 GetQueueFamilyIndex() const { return m_queue_family_index; }

		private:
			VkCommandPool m_handle;

			u32 m_queue_family_index;
		};

		class VulkanCommandBuffer
		{
		public:
			VulkanCommandBuffer();
			~VulkanCommandBuffer();

			void Allocate(const VulkanDevice& device, const VulkanCommandPool& command_pool, VkCommandBufferLevel level);
			void Free(const VulkanDevice& device, const VulkanCommandPool& command_pool);
			void Reset(VkCommandBufferResetFlags flags = 0) const;

			void BeginRecording(VkCommandBufferUsageFlags flags = 0) const;
			void EndRecording() const;

			void BeginRecordingOneTime(const VulkanDevice& device, const VulkanCommandPool& command_pool) const;
			void EndRecordingOneTime(const VulkanDevice& device, const VulkanCommandPool& command_pool) const;

			VkCommandBuffer GetHandle() const { return m_handle; }

		private:
			VkCommandBuffer m_handle;
		};

	} // namespace graphics

} // namespace hellengine