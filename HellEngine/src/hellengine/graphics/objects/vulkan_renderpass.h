#pragma once

// Internal
#include "shared.h"
#include "vulkan_framebuffer.h"

namespace hellengine
{

	namespace graphics
	{

		class VulkanRenderPass
		{
		public:
			VulkanRenderPass();
			~VulkanRenderPass();

			void Create(const VulkanInstance& instance, const VulkanDevice& device, VulkanSwapchain& swapchain, const RenderPassInfo& info);
			void Destroy(const VulkanInstance& instance, const VulkanDevice& device);

			void RecreateFramebuffers(const VulkanInstance& instance, const VulkanDevice& device, VulkanSwapchain& swapchain, const RenderPassInfo& info);
			
			VkRenderPass GetHandle() const { return m_handle; }

			INLINE VulkanFramebuffer& GetFramebuffer(u32 index) { return m_framebuffers[index]; }
			INLINE const VulkanFramebuffer& GetFramebuffer(u32 index) const { return m_framebuffers[index]; }

		private:
			void CreateFramebuffers(const VulkanInstance& instance, const VulkanDevice& device, VulkanSwapchain& swapchain, const RenderPassInfo& info);
			void DestroyFramebuffers(const VulkanInstance& instance, const VulkanDevice& device);

		private:
			VkRenderPass m_handle;

			std::vector<VulkanFramebuffer> m_framebuffers;
		};

	} // graphics

} // namespace hellengine