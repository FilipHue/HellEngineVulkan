#pragma once

// Internal
#include "shared.h"
#include "vulkan_image.h"

namespace hellengine
{

	namespace graphics
	{

		struct FrameData;

		class VulkanSwapchain
		{
		public:
			VulkanSwapchain();
			~VulkanSwapchain();

			void Create(const VulkanInstance& instance, VulkanDevice& device, VulkanSurface& surface, VkExtent2D current_extent);
			void Destroy(const VulkanInstance& instance, const VulkanDevice& device);
			void Recreate(const VulkanInstance& instance, VulkanDevice& device, VulkanSurface& surface);

			void CreateImageViews(const VulkanInstance& instance, VulkanDevice& device);
			void DestroyImageViews(const VulkanInstance& instance, const VulkanDevice& device);

			b8 AcquireNextImage(const VulkanInstance& instance, VulkanDevice& device, VulkanSurface& surface, FrameData& frame_data);
			void Submit(const VulkanDevice& device, VulkanSurface& surface, FrameData& frame_data);
			b8 Present(const VulkanInstance& instance, VulkanDevice& device, VulkanSurface& surface, FrameData& frame_data);

			VkSwapchainKHR GetHandle() const { return m_handle; }
			VkSurfaceFormatKHR GetSurfaceFormat() const { return m_format; }
			VkFormat GetSwapchainImageFormat() const { return m_format.format; }
			VkExtent2D GetExtent() const { return m_extent; }
			std::vector<VkImage>& GetImages() { return m_images; }
			const std::vector<VkImage>& GetImages() const { return m_images; }
			std::vector<VkImageView>& GetImageViews() { return m_image_views; }
			const std::vector<VkImageView>& GetImageViews() const { return m_image_views; }

			VulkanImage& GetDepthImage() { return m_depth_image; }
			const VulkanImage& GetDepthImage() const { return m_depth_image; }
			VulkanImage& GetStencilImage() { return m_stencil_image; }
			const VulkanImage& GetStencilImage() const { return m_stencil_image; }
			VulkanImage& GetDepthStencilImage() { return m_depth_stencil_image; }
			const VulkanImage& GetDepthStencilImage() const { return m_depth_stencil_image; }

			u32 GetFramesInFlight() const { return m_frames_in_flight; }

			void SetFramesInFlight(u32 frames_in_flight) { m_frames_in_flight = frames_in_flight; }
			void SetExtent(VkExtent2D extent) { m_extent = extent; }

			void SetResizeCallback(std::function<void()> callback) { m_resize_callback = callback; }

		private:
			void Init(const VulkanInstance& instance, VulkanDevice& device, VulkanSurface& surface, VkExtent2D current_extent);
			void Cleanup(const VulkanInstance& instance, const VulkanDevice& device);

		private:
			VkSwapchainKHR m_handle;
			VkSurfaceFormatKHR m_format;
			VkExtent2D m_extent;
			std::vector<VkImage> m_images;
			std::vector<VkImageView> m_image_views;

			VulkanImage m_depth_image;
			VulkanImage m_stencil_image;
			VulkanImage m_depth_stencil_image;

			u32 m_frames_in_flight;

			std::function<void()> m_resize_callback;
		};

		static VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& available_formats);
		static VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& available_present_modes);
		static VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, VkExtent2D current_extent);

	} // namespace graphics

} // namespace hellengine