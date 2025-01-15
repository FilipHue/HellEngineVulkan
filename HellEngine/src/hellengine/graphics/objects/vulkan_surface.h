#pragma once

// Internal
#include "shared.h"

namespace hellengine
{

	namespace core
	{
		class Window;
	}

	namespace graphics
	{

		class VulkanSurface
		{
		public:
			VulkanSurface();
			~VulkanSurface();

			void Create(const VulkanInstance& instance, const core::Window* window);
			void Destroy(const VulkanInstance& instance);

			VkSurfaceKHR GetHandle() const { return m_handle; }

			void AddSwapChainSupportDetails(SwapChainSupportDetails details) { m_swapchain_support_details = details; }

			SwapChainSupportDetails& GetSwapChainSupportDetails() { return m_swapchain_support_details; }
			const SwapChainSupportDetails& GetSwapChainSupportDetails() const { return m_swapchain_support_details; }

			VkSurfaceCapabilitiesKHR GetCapabilities() const { return m_swapchain_support_details.capabilities; }
			const std::vector<VkSurfaceFormatKHR>& GetFormats() const { return m_swapchain_support_details.formats; }
			const std::vector<VkPresentModeKHR>& GetPresentModes() const { return m_swapchain_support_details.present_modes; }

		private:
			VkSurfaceKHR m_handle;

			SwapChainSupportDetails m_swapchain_support_details;
		};

	} // namespace graphics

} // namespace hellengine