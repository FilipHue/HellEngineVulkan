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

			void Create(const VulkanInstance& instance, const VulkanDevice& device, VkImageType image_type, VkFormat format, VkExtent3D extent, u32 mip_levels, u32 array_layers, VkSampleCountFlagBits sample_count, VkImageTiling tiling, VkImageUsageFlags usage, VkImageCreateFlags flags, VkMemoryPropertyFlags properties);
			void CreateImageView(const VulkanInstance& instance, const VulkanDevice& device, VkImageViewType view_type, VkFormat format, VkImageSubresourceRange range);
			void Destroy(const VulkanInstance& instance, const VulkanDevice& device) const;

			VkImage GetHandle() const { return m_handle; }
			VkImageView GetImageView() const { return m_image_view; }
			VkDeviceMemory GetMemory() const { return m_memory; }

			u32 GetWidth() const { return m_width; }
			u32 GetHeight() const { return m_height; }
			u32 GetDepth() const { return m_depth; }

			static void Transition(const VulkanDevice& device, const VulkanCommandPool& command_pool, VkImage image, VkImageSubresourceRange subresource, VkImageLayout old_layout, VkImageLayout new_layout);
			static void PrepareTransition(const VulkanCommandBuffer& command_buffer, VkImage image, VkImageSubresourceRange subresource, VkImageLayout old_layout, VkImageLayout new_layout);


		private:
			VkImage m_handle;
			VkImageView m_image_view;
			VkDeviceMemory m_memory;

			u32 m_width;
			u32 m_height;
			u32 m_depth;
		};

		void CreateImageView(const VulkanInstance& instance, const VulkanDevice& device, VkImage image, VkImageViewType view_type, VkFormat format, VkImageSubresourceRange range, VkImageView& image_view);
		void DestroyImageView(const VulkanInstance& instance, const VulkanDevice& device, VkImageView& image_view);

		static VkAccessFlags GetAccessMask(VkImageLayout layout);
		static VkPipelineStageFlags GetPipelineStageFlags(VkImageLayout layout);

	} // namespace graphics

} // namespace hellengine