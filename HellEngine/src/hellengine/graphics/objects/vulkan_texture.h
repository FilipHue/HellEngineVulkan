#pragma once

// Internal
#include "shared.h"

namespace hellengine
{

	namespace graphics
	{

		class VulkanTexture
		{
		public:
			VulkanTexture();
			~VulkanTexture();

			void Create(const VulkanInstance& instance, const VulkanDevice& device, const VulkanCommandPool& command_pool, const char* path);
			void Destroy(const VulkanInstance& instance, const VulkanDevice& device) const;

			VkImage GetHandle() const { return m_handle; }
			VkImageView GetImageView() const { return m_image_view; }
			VkSampler GetSampler() const { return m_sampler; }

		private:
			void CreateImage(const VulkanInstance& instance, const VulkanDevice& device);
			void AllocateDeviceMemory(const VulkanInstance& instance, const VulkanDevice& device);
			void CreateSampler(const VulkanInstance& instance, const VulkanDevice& device);

		private:
			VkImage m_handle;
			VkImageView m_image_view;
			VkDeviceMemory m_image_memory;
			VkSampler m_sampler;

			i32 m_width;
			i32 m_height;
			i32 m_channels;
		};

	} // namespace graphics

} // namespace hellengine