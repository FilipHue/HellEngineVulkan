#pragma once

// Internal
#include "shared.h"
#include "vulkan_image.h"

namespace hellengine
{

	namespace graphics
	{

		class VulkanTexture
		{
		public:
			VulkanTexture();
			virtual ~VulkanTexture();

			void Update(const VulkanInstance& instance, const VulkanDevice& device, const VulkanCommandPool& command_pool, const void* data);

			void Destroy(const VulkanInstance& instance, const VulkanDevice& device);

			VkImage GetHandle() const { return m_image.GetHandle(); }
			VkImageView GetImageView() const { return m_image.GetImageView(); }
			VkSampler GetSampler() const { return m_sampler; }
			VkFormat GetFormat() const { return m_format; }

			u32 GetWidth() const { return m_image.GetWidth(); }
			u32 GetHeight() const { return m_image.GetHeight(); }
			u32 GetDepth() const { return m_image.GetDepth(); }
			u32 GetChannels() const { return m_channels; }
			u32 GetMipLevels() const { return m_mip_levels; }
			u32 GetLayerCount() const { return m_layer_count; }
			u32 GetFaces() const { return m_faces; }

			static void CreateSampler(const VulkanInstance& instance, const VulkanDevice& device, const ImageSamplerCreationInfo& info, VkSampler* sampler);

		protected:
			VulkanImage m_image;
			VkSampler m_sampler;
			VkFormat m_format;

			u32 m_channels;
			u32 m_mip_levels;
			u32 m_layer_count;
			u32 m_faces;
		};

		class VulkanTexture2D : public VulkanTexture
		{
		public:
			VulkanTexture2D();
			~VulkanTexture2D();

			void Create(const VulkanInstance& instance, const VulkanDevice& device, const VulkanCommandPool& command_pool, VkFormat format, i32 width, i32 height);
			void Create(const VulkanInstance& instance, const VulkanDevice& device, const VulkanCommandPool& command_pool, VkFormat format, const void* data, i32 width, i32 height);
			void Create(const VulkanInstance& instance, const VulkanDevice& device, const VulkanCommandPool& command_pool, VkFormat format, const char* path);

			void LoadSTBI(const VulkanInstance& instance, const VulkanDevice& device, const VulkanCommandPool& command_pool, VkFormat format, const char* path);
		};

		class VulkanTexture3D : public VulkanTexture
		{
		public:
			VulkanTexture3D();
			~VulkanTexture3D();

			void Create(const VulkanInstance& instance, const VulkanDevice& device, const VulkanCommandPool& command_pool, VkFormat format, i32 width, i32 height, i32 depth);
			void Create(const VulkanInstance& instance, const VulkanDevice& device, const VulkanCommandPool& command_pool, VkFormat format, const void* data, i32 width, i32 height, i32 depth);
		};

		class VulkanTextureCubemap : public VulkanTexture
		{
		public:
			VulkanTextureCubemap();
			~VulkanTextureCubemap();

			void Create(const VulkanInstance& instance, const VulkanDevice& device, const VulkanCommandPool& command_pool, VkFormat format, const char* path);
			void CreateArray(const VulkanInstance& instance, const VulkanDevice& device, const VulkanCommandPool& command_pool, VkFormat format, const char* path);
		};

		VkImageAspectFlags GetAspectMaskFromVkFormat(VkFormat format);
		VkImageUsageFlags GetUsageFlagsFromVkFormat(VkFormat format);


	} // namespace graphics

} // namespace hellengine