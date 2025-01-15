#pragma once

// Internal
#include "shared.h"

namespace hellengine
{

	namespace graphics
	{

		class VulkanSemaphore
		{
		public:
			VulkanSemaphore();
			~VulkanSemaphore();

			void Create(const VulkanInstance& instance, const VulkanDevice& device);
			void Destroy(const VulkanInstance& instance, const VulkanDevice& device) const;

			VkSemaphore GetHandle() const { return m_handle; }

		private:
			VkSemaphore m_handle;
		};

		class VulkanFence
		{
		public:
			VulkanFence();
			~VulkanFence();

			void Create(const VulkanInstance& instance, const VulkanDevice& device, b8 signaled = false);
			void Destroy(const VulkanInstance& instance, const VulkanDevice& device) const;

			VkFence GetHandle() const { return m_handle; }

			static void Reset(const VulkanDevice& device, VkFence* fences, u32 count);
			static void Wait(const VulkanDevice& device, VkFence* fences, u32 count, b8 wait_all = true, u64 timeout = UINT64_MAX);

		private:
			VkFence m_handle;

			b8 m_signaled;
		};

	} // namespace graphics

} // namespace hellengine