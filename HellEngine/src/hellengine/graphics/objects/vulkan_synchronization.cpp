#include "hepch.h"
#include "vulkan_synchronization.h"

// Internal
#include "vulkan_instance.h"
#include "vulkan_device.h"

namespace hellengine
{

	namespace graphics
	{

		VulkanSemaphore::VulkanSemaphore()
		{
			m_handle = VK_NULL_HANDLE;
		}

		VulkanSemaphore::~VulkanSemaphore()
		{
			NO_OP;
		}

		void VulkanSemaphore::Create(const VulkanInstance& instance, const VulkanDevice& device)
		{
			VkSemaphoreCreateInfo semaphore_info = {};
			semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
			semaphore_info.pNext = VK_NULL_HANDLE;
			semaphore_info.flags = 0;

			VK_CHECK(vkCreateSemaphore(device.GetLogicalDevice(), &semaphore_info, instance.GetAllocator(), &m_handle));
		}

		void VulkanSemaphore::Destroy(const VulkanInstance& instance, const VulkanDevice& device) const
		{
			vkDestroySemaphore(device.GetLogicalDevice(), m_handle, instance.GetAllocator());
		}

		VulkanFence::VulkanFence()
		{
			m_handle = VK_NULL_HANDLE;
			m_signaled = false;
		}

		VulkanFence::~VulkanFence()
		{
			NO_OP;
		}

		void VulkanFence::Create(const VulkanInstance& instance, const VulkanDevice& device, b8 signaled)
		{
			m_signaled = signaled;

			VkFenceCreateInfo fence_info = {};
			fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
			fence_info.pNext = VK_NULL_HANDLE;
			fence_info.flags = signaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0;

			VK_CHECK(vkCreateFence(device.GetLogicalDevice(), &fence_info, instance.GetAllocator(), &m_handle));
		}

		void VulkanFence::Destroy(const VulkanInstance& instance, const VulkanDevice& device) const
		{
			vkDestroyFence(device.GetLogicalDevice(), m_handle, instance.GetAllocator());
		}

		void VulkanFence::Reset(const VulkanDevice& device, VkFence* fences, u32 count)
		{
			VK_CHECK(vkResetFences(device.GetLogicalDevice(), count, fences));
		}

		void VulkanFence::Wait(const VulkanDevice& device, VkFence* fences, u32 count, b8 wait_all, u64 timeout)
		{
			VK_CHECK(vkWaitForFences(device.GetLogicalDevice(), count, fences, wait_all, timeout));
		}

	} // namespace graphics

} // namespace hellengine