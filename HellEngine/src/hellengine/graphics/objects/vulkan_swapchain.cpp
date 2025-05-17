#include "hepch.h"
#include "vulkan_swapchain.h"

// Internal
#include "vulkan_instance.h"
#include "vulkan_device.h"
#include "vulkan_surface.h"

#include <hellengine/graphics/context.h>

namespace hellengine
{

	namespace graphics
	{

		VulkanSwapchain::VulkanSwapchain()
		{
			m_handle = VK_NULL_HANDLE;
			m_format = {};
			m_extent = {};
			m_images = {};
			m_image_views = {};

			m_frames_in_flight = MAX_FRAMES_IN_FLIGHT;
		}

		VulkanSwapchain::~VulkanSwapchain()
		{
			NO_OP;
		}

		void VulkanSwapchain::Create(const VulkanInstance& instance, VulkanDevice& device, VulkanSurface& surface, VkExtent2D current_extent)
		{
			HE_GRAPHICS_DEBUG("Creating vulkan swapchain...");
			Init(instance, device, surface, current_extent);
			HE_GRAPHICS_DEBUG("Vulkan swapchain created successfully");
		}

		void VulkanSwapchain::Destroy(const VulkanInstance& instance, const VulkanDevice& device)
		{
			HE_GRAPHICS_DEBUG("Destroying vulkan swapchain...");
			Cleanup(instance, device);
			HE_GRAPHICS_DEBUG("Vulkan swapchain destroyed successfully");
		}

		void VulkanSwapchain::Recreate(const VulkanInstance& instance, VulkanDevice& device, VulkanSurface& surface)
		{
			HE_GRAPHICS_DEBUG("Recreating vulkan swapchain...");
			Cleanup(instance, device);
			Init(instance, device, surface, m_extent);
			HE_GRAPHICS_DEBUG("Vulkan swapchain recreated successfully");
		}

		void VulkanSwapchain::CreateImageViews(const VulkanInstance& instance, VulkanDevice& device)
		{
			for (VkImage image : m_images)
			{
				VkImageView image_view{};
				CreateImageView(instance, device, image, VK_IMAGE_VIEW_TYPE_2D, m_format.format, { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 }, image_view);
				m_image_views.push_back(image_view);
			}
		}

		void VulkanSwapchain::DestroyImageViews(const VulkanInstance& instance, const VulkanDevice& device)
		{
			for (VkImageView image_view : m_image_views)
			{
				DestroyImageView(instance, device, image_view);
			}

			m_image_views.clear();
		}

		b8 VulkanSwapchain::AcquireNextImage(const VulkanInstance& instance, VulkanDevice& device, VulkanSurface& surface, FrameData& frame_data)
		{
			u32 count = 1;
			VkFence fences[] = { frame_data.frame_in_flight.GetHandle() };
			VulkanFence::Wait(device, fences, count);

			VkResult result = vkAcquireNextImageKHR(device.GetLogicalDevice(), m_handle, UINT64_MAX, frame_data.image_available.GetHandle(), VK_NULL_HANDLE, &frame_data.image_index);

			if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || frame_data.framebuffer_resized)
			{
				frame_data.framebuffer_resized = false;
				m_resize_callback();

				return false;
			}
			VulkanFence::Reset(device, fences, count);

			frame_data.command_buffer.Reset();
			frame_data.command_buffer.BeginRecording();

			return true;
		}

		void VulkanSwapchain::Submit(const VulkanDevice& device, VulkanSurface& surface, FrameData& frame_data)
		{
			frame_data.command_buffer.EndRecording();

			VkSemaphore wait_semaphores[] = { frame_data.image_available.GetHandle() };
			VkSemaphore signal_semaphores[] = { frame_data.render_finished.GetHandle() };
			VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
			VkCommandBuffer command_buffers[] = { frame_data.command_buffer.GetHandle() };

			VkSubmitInfo submit_info = {};
			submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submit_info.pNext = VK_NULL_HANDLE;
			submit_info.waitSemaphoreCount = 1;
			submit_info.pWaitSemaphores = wait_semaphores;
			submit_info.pWaitDstStageMask = wait_stages;
			submit_info.commandBufferCount = 1;
			submit_info.pCommandBuffers = command_buffers;
			submit_info.signalSemaphoreCount = 1;
			submit_info.pSignalSemaphores = signal_semaphores;

			VK_CHECK(vkQueueSubmit(device.GetGraphicsQueue(), 1, &submit_info, frame_data.frame_in_flight.GetHandle()));
		}

		b8 VulkanSwapchain::Present(const VulkanInstance& instance, VulkanDevice& device, VulkanSurface& surface, FrameData& frame_data)
		{
			VkSemaphore wait_semaphores[] = { frame_data.render_finished.GetHandle() };
			VkSwapchainKHR swapchains[] = { m_handle };

			VkPresentInfoKHR present_info = {};
			present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
			present_info.pNext = VK_NULL_HANDLE;
			present_info.waitSemaphoreCount = 1;
			present_info.pWaitSemaphores = wait_semaphores;
			present_info.swapchainCount = 1;
			present_info.pSwapchains = swapchains;
			present_info.pImageIndices = &frame_data.image_index;

			VkResult result = vkQueuePresentKHR(device.GetPresentQueue(), &present_info);

			if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || frame_data.framebuffer_resized)
			{
				frame_data.framebuffer_resized = false;
				m_resize_callback();
				return false;
			}

			return true;
		}

		void VulkanSwapchain::Init(const VulkanInstance& instance, VulkanDevice& device, VulkanSurface& surface, VkExtent2D current_extent)
		{
			ValidateSwapChainSupport(device.GetPhysicalDevice(), surface);

			VkSurfaceFormatKHR surface_format = ChooseSwapSurfaceFormat(surface.GetFormats());
			VkPresentModeKHR present_mode = ChooseSwapPresentMode(surface.GetPresentModes());
			VkExtent2D extent = ChooseSwapExtent(surface.GetCapabilities(), current_extent);

			u32 image_count = surface.GetCapabilities().minImageCount + 1;
			if (surface.GetCapabilities().maxImageCount > 0 && image_count > surface.GetCapabilities().maxImageCount)
			{
				image_count = surface.GetCapabilities().maxImageCount;
			}

			m_format = surface_format;
			m_extent = extent;

			VkSwapchainCreateInfoKHR create_info = {};
			create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
			create_info.surface = surface.GetHandle();
			create_info.minImageCount = image_count;
			create_info.imageFormat = surface_format.format;
			create_info.imageColorSpace = surface_format.colorSpace;
			create_info.imageExtent = extent;
			create_info.imageArrayLayers = 1;
			create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

			u32 queue_family_indices[] = { device.GetQueueFamilyIndices().graphics_family.value(), device.GetQueueFamilyIndices().present_family.value() };

			if (device.GetQueueFamilyIndices().graphics_family != device.GetQueueFamilyIndices().present_family)
			{
				create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
				create_info.queueFamilyIndexCount = 2;
				create_info.pQueueFamilyIndices = queue_family_indices;
			}
			else
			{
				create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
				create_info.queueFamilyIndexCount = 0;
				create_info.pQueueFamilyIndices = VK_NULL_HANDLE;
			}

			create_info.preTransform = surface.GetCapabilities().currentTransform;
			create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
			create_info.presentMode = present_mode;
			create_info.clipped = VK_TRUE;
			create_info.oldSwapchain = VK_NULL_HANDLE;

			VK_CHECK(vkCreateSwapchainKHR(device.GetLogicalDevice(), &create_info, instance.GetAllocator(), &m_handle));

			VK_CHECK(vkGetSwapchainImagesKHR(device.GetLogicalDevice(), m_handle, &image_count, nullptr));
			m_images.resize(image_count);
			VK_CHECK(vkGetSwapchainImagesKHR(device.GetLogicalDevice(), m_handle, &image_count, m_images.data()));

			CreateImageViews(instance, device);

			if (!DetectDepthFormat(device))
			{
				device.SetDepthFormat(VK_FORMAT_UNDEFINED);
				HE_CORE_CRITICAL("Failed to detect depth format");
			}

			m_depth_image.Create(instance, device, VK_IMAGE_TYPE_2D, device.GetDepthFormat(), { m_extent.width, m_extent.height, 1 }, 1, 1, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, 0, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
			m_depth_image.CreateImageView(instance, device, VK_IMAGE_VIEW_TYPE_2D, device.GetDepthFormat(), { VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1 });

			if (!DetectStencilFormat(device))
			{
				device.SetStencilFormat(VK_FORMAT_UNDEFINED);
				HE_CORE_CRITICAL("Failed to detect stencil format");
			}

			m_stencil_image.Create(instance, device, VK_IMAGE_TYPE_2D, device.GetStencilFormat(), { m_extent.width, m_extent.height, 1 }, 1, 1, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, 0, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
			m_stencil_image.CreateImageView(instance, device, VK_IMAGE_VIEW_TYPE_2D, device.GetStencilFormat(), { VK_IMAGE_ASPECT_STENCIL_BIT, 0, 1, 0, 1 });

			if (!DetectDepthStencilFormat(device))
			{
				device.SetDepthStencilFormat(VK_FORMAT_UNDEFINED);
				HE_CORE_CRITICAL("Failed to detect depth stencil format");
			}

			m_depth_stencil_image.Create(instance, device, VK_IMAGE_TYPE_2D, device.GetDepthStencilFormat(), { m_extent.width, m_extent.height, 1 }, 1, 1, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, 0, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
			m_depth_stencil_image.CreateImageView(instance, device, VK_IMAGE_VIEW_TYPE_2D, device.GetDepthStencilFormat(), { VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, 0, 1, 0, 1 });
		}

		void VulkanSwapchain::Cleanup(const VulkanInstance& instance, const VulkanDevice& device)
		{
			m_depth_image.Destroy(instance, device);
			m_stencil_image.Destroy(instance, device);
			m_depth_stencil_image.Destroy(instance, device);
			DestroyImageViews(instance, device);
			vkDestroySwapchainKHR(device.GetLogicalDevice(), m_handle, instance.GetAllocator());
		}

		VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& available_formats)
		{
			for (const auto& available_format : available_formats)
			{
				if (available_format.format == VK_FORMAT_B8G8R8A8_UNORM && available_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
				{
					return available_format;
				}
			}

			return available_formats[0];
		}

		VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& available_present_modes)
		{
			for (const auto& available_present_mode : available_present_modes)
			{
				if (available_present_mode == VK_PRESENT_MODE_MAILBOX_KHR)
				{
					return available_present_mode;
				}
			}

			return VK_PRESENT_MODE_FIFO_KHR;
		}

		VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, VkExtent2D current_extent)
		{
			if (capabilities.currentExtent.width != U32MAX)
			{
				return capabilities.currentExtent;
			}
			else
			{
				VkExtent2D actual_extent = { current_extent.width, current_extent.height };

				actual_extent.width = CLAMP(actual_extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
				actual_extent.height = CLAMP(actual_extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

				return actual_extent;
			}
		}

	} // namespace graphics

} // namespace hellengine