#pragma once

// Internal
#include "shared.h"

namespace hellengine
{

	namespace graphics
	{

		class VulkanImage
		{
		public:
			VulkanImage();
			~VulkanImage();

			void Create(const VulkanInstance& instance, VulkanDevice& device, VkImageType image_type, VkFormat format, u32 width, u32 height, u32 depth, VkImageUsageFlags usage, VkImageTiling tiling, VkMemoryPropertyFlags properties);
			void Destroy(const VulkanInstance& instance, const VulkanDevice& device) const;

			VkImage GetHandle() const { return m_handle; }
			VkImageView& GetImageView() { return m_image_view; }

			u32 GetWidth() const { return m_width; }
			u32 GetHeight() const { return m_height; }
			u32 GetDepth() const { return m_depth; }

			static void Transition(const VulkanDevice& device, const VulkanCommandPool& command_pool, VkImage image, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout);

		private:
			VkImage m_handle;
			VkImageView m_image_view;
			VkDeviceMemory m_memory;

			u32 m_width;
			u32 m_height;
			u32 m_depth;
		};

		void CreateImageView(const VulkanInstance& instance, const VulkanDevice& device, VkImage image, VkFormat format, VkImageAspectFlags aspect_flags, VkImageView& image_view);
		void DestroyImageView(const VulkanInstance& instance, const VulkanDevice& device, VkImageView image_view);

	} // namespace graphics

} // namespace hellengine