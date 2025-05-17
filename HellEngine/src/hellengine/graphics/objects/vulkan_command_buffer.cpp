#include "hepch.h"
#include "vulkan_command_buffer.h"

// Internal
#include "vulkan_instance.h"
#include "vulkan_device.h"

namespace hellengine
{

	namespace graphics
	{

		VulkanCommandPool::VulkanCommandPool()
		{
			m_handle = VK_NULL_HANDLE;
			m_queue_family_index = U32MAX;
		}

		VulkanCommandPool::~VulkanCommandPool()
		{
			NO_OP;
		}

		void VulkanCommandPool::Create(const VulkanInstance& instance, const VulkanDevice& device, u32 queue_family_index)
		{
			m_queue_family_index = queue_family_index;

			VkCommandPoolCreateInfo pool_info = {};
			pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			pool_info.pNext = VK_NULL_HANDLE;
			pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
			pool_info.queueFamilyIndex = queue_family_index;

			VK_CHECK(vkCreateCommandPool(device.GetLogicalDevice(), &pool_info, instance.GetAllocator(), &m_handle));
		}

		void VulkanCommandPool::Destroy(const VulkanInstance& instance, const VulkanDevice& device) const
		{
			vkDestroyCommandPool(device.GetLogicalDevice(), m_handle, instance.GetAllocator());
		}

		VulkanCommandBuffer::VulkanCommandBuffer()
		{
			m_handle = VK_NULL_HANDLE;
		}

		VulkanCommandBuffer::~VulkanCommandBuffer()
		{
			NO_OP;
		}

		void VulkanCommandBuffer::Allocate(const VulkanDevice& device, const VulkanCommandPool& command_pool, VkCommandBufferLevel level)
		{
			VkCommandBufferAllocateInfo alloc_info = {};
			alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			alloc_info.pNext = VK_NULL_HANDLE;
			alloc_info.commandPool = command_pool.GetHandle();
			alloc_info.level = level;
			alloc_info.commandBufferCount = 1;

			VK_CHECK(vkAllocateCommandBuffers(device.GetLogicalDevice(), &alloc_info, &m_handle));
		}

		void VulkanCommandBuffer::Free(const VulkanDevice& device, const VulkanCommandPool& command_pool)
		{
			vkFreeCommandBuffers(device.GetLogicalDevice(), command_pool.GetHandle(), 1, &m_handle);
			m_handle = VK_NULL_HANDLE;
		}

		void VulkanCommandBuffer::Reset(VkCommandBufferResetFlags flags) const
		{
			VK_CHECK(vkResetCommandBuffer(m_handle, flags));
		}

		void VulkanCommandBuffer::BeginRecording(VkCommandBufferUsageFlags flags) const
		{
			VkCommandBufferBeginInfo begin_info = {};
			begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			begin_info.pNext = VK_NULL_HANDLE;
			begin_info.flags = flags;
			begin_info.pInheritanceInfo = VK_NULL_HANDLE;

			VK_CHECK(vkBeginCommandBuffer(m_handle, &begin_info));
		}

		void VulkanCommandBuffer::EndRecording() const
		{
			VK_CHECK(vkEndCommandBuffer(m_handle));
		}

		void VulkanCommandBuffer::BeginRecordingOneTime(const VulkanDevice& device, const VulkanCommandPool& command_pool) const
		{
			VkCommandBufferBeginInfo begin_info = {};
			begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			begin_info.pNext = VK_NULL_HANDLE;
			begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
			begin_info.pInheritanceInfo = VK_NULL_HANDLE;

			VK_CHECK(vkBeginCommandBuffer(m_handle, &begin_info));
		}

		void VulkanCommandBuffer::EndRecordingOneTime(const VulkanDevice& device, const VulkanCommandPool& command_pool) const
		{
			VK_CHECK(vkEndCommandBuffer(m_handle));

			VkSubmitInfo submit_info = {};
			submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submit_info.pNext = VK_NULL_HANDLE;
			submit_info.waitSemaphoreCount = 0;
			submit_info.pWaitSemaphores = VK_NULL_HANDLE;
			submit_info.pWaitDstStageMask = VK_NULL_HANDLE;
			submit_info.commandBufferCount = 1;
			submit_info.pCommandBuffers = &m_handle;
			submit_info.signalSemaphoreCount = 0;
			submit_info.pSignalSemaphores = VK_NULL_HANDLE;

			VK_CHECK(vkQueueSubmit(device.GetGraphicsQueue(), 1, &submit_info, VK_NULL_HANDLE));
			VK_CHECK(vkQueueWaitIdle(device.GetGraphicsQueue()));
		}

	} // namespace graphics

} // namespace hellengine