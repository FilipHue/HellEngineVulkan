#include "hepch.h"
#include "vulkan_texture.h"

// Internal
#include "vulkan_instance.h"
#include "vulkan_device.h"
#include "vulkan_command_buffer.h"
#include "vulkan_buffer.h"
#include "vulkan_image.h"

// External
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include "ktx.h"
#include "ktxvulkan.h"

namespace hellengine
{

	namespace graphics
	{

		VulkanTexture::VulkanTexture()
		{
			m_image = VulkanImage();
			m_sampler = VK_NULL_HANDLE;
			m_format = VK_FORMAT_UNDEFINED;

			m_channels = 0;
			m_mip_levels = 0;
			m_layer_count = 0;
			m_faces = 0;
		}

		VulkanTexture::~VulkanTexture()
		{
			NO_OP;
		}

		void VulkanTexture::Update(const VulkanInstance& instance, const VulkanDevice& device, const VulkanCommandPool& command_pool, const void* data)
		{
			VulkanBuffer stagging_buffer;
			stagging_buffer.Create(instance, device, m_image.GetWidth() * m_image.GetHeight() * m_image.GetDepth() * m_channels, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, false, 0, nullptr, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
			stagging_buffer.MapUnmap(device, m_image.GetWidth() * m_image.GetHeight() * m_image.GetDepth() * m_channels, 0, data);

			std::vector<VkBufferImageCopy> buffer_copy_regions;
			VkBufferImageCopy region = {};

			region.bufferOffset = 0;
			region.bufferRowLength = 0;
			region.bufferImageHeight = 0;

			region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			region.imageSubresource.mipLevel = 0;
			region.imageSubresource.baseArrayLayer = 0;
			region.imageSubresource.layerCount = 1;

			region.imageOffset = { 0, 0, 0 };
			region.imageExtent = { m_image.GetWidth(), m_image.GetHeight(), m_image.GetDepth() };

			buffer_copy_regions.push_back(region);

			VulkanImage::Transition(device, command_pool, m_image.GetHandle(), { VK_IMAGE_ASPECT_COLOR_BIT, 0, m_mip_levels, 0, m_layer_count }, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
			stagging_buffer.CopyToImage(device, command_pool, m_image.GetHandle(), buffer_copy_regions);
			VulkanImage::Transition(device, command_pool, m_image.GetHandle(), { VK_IMAGE_ASPECT_COLOR_BIT, 0, m_mip_levels, 0, m_layer_count }, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

			stagging_buffer.Destroy(instance, device);
		}

		void VulkanTexture::Destroy(const VulkanInstance& instance, const VulkanDevice& device)
		{
			if (m_sampler != VK_NULL_HANDLE)
			{
				vkDestroySampler(device.GetLogicalDevice(), m_sampler, instance.GetAllocator());
				m_sampler = VK_NULL_HANDLE;
			}
			m_image.Destroy(instance, device);
		}

		template u32 VulkanTexture::ReadPixel<u32>(const VulkanInstance& instance, const VulkanDevice& device, const VulkanCommandPool& command_pool, u32 x, u32 y, u32 layer, u32 face);
		template <typename T>
		T VulkanTexture::ReadPixel(const VulkanInstance& instance, const VulkanDevice& device, const VulkanCommandPool& command_pool, u32 x, u32 y, u32 layer, u32 face)
		{
			if (x >= m_image.GetWidth() || y >= m_image.GetHeight() || layer >= m_layer_count || face >= m_faces)
			{
				HE_GRAPHICS_ERROR("Attempted to read pixel out of bounds: ({}, {}) layer {}, face {} for texture with dimensions {}x{}x{} and {} layers and {} faces", x, y, layer, face, m_image.GetWidth(), m_image.GetHeight(), m_image.GetDepth(), m_layer_count, m_faces);
			}

			T pixel_data;
			VkDeviceSize size = sizeof(T);

			VulkanBuffer staging_buffer;
			staging_buffer.Create(instance, device, size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT, false, 0, nullptr, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

			std::vector<VkBufferImageCopy> buffer_ranges = {
				{
					.bufferOffset = 0,
					.bufferRowLength = 0,
					.bufferImageHeight = 0,
					.imageSubresource = { GetAspectMaskFromVkFormat(m_format), face, layer, 1 },
					.imageOffset = { (i32)x, (i32)y, 0 },
					.imageExtent = { 1, 1, 1 }
				}
			};

			VulkanImage::Transition(device, command_pool, m_image.GetHandle(), { VK_IMAGE_ASPECT_COLOR_BIT, 0, m_mip_levels, 0, m_layer_count }, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

			staging_buffer.CopyFromImage(device, command_pool, m_image.GetHandle(), buffer_ranges);

			VulkanImage::Transition(device, command_pool, m_image.GetHandle(), { VK_IMAGE_ASPECT_COLOR_BIT, 0, m_mip_levels, 0, m_layer_count }, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

			void* data = nullptr;
			staging_buffer.Map(device, size, 0, data);
			memcpy(&pixel_data, data, size);
			staging_buffer.Unmap(device);
			staging_buffer.Destroy(instance, device);

			return pixel_data;
		}

		void VulkanTexture::CreateSampler(const VulkanInstance& instance, const VulkanDevice& device, const ImageSamplerCreationInfo& info, VkSampler* sampler)
		{
			VkSamplerCreateInfo sampler_info = {};
			sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
			sampler_info.pNext = VK_NULL_HANDLE;
			sampler_info.flags = 0;
			sampler_info.magFilter = GetVulkanFilter(info.mag_filter);
			sampler_info.minFilter = GetVulkanFilter(info.mag_filter);
			sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
			sampler_info.addressModeU = GetVulkanAddressMode(info.address_mode[0]);
			sampler_info.addressModeV = GetVulkanAddressMode(info.address_mode[1]);
			sampler_info.addressModeW = GetVulkanAddressMode(info.address_mode[2]);
			sampler_info.mipLodBias = 0.0f;

			sampler_info.anisotropyEnable = VK_TRUE;
			VkPhysicalDeviceProperties properties = {};
			vkGetPhysicalDeviceProperties(device.GetPhysicalDevice(), &properties);
			sampler_info.maxAnisotropy = properties.limits.maxSamplerAnisotropy;

			sampler_info.compareEnable = info.compare_op.has_value() ? VK_TRUE : VK_FALSE;
			sampler_info.compareOp = info.compare_op.has_value() ? GetVulkanCompareOp(info.compare_op.value()) : VK_COMPARE_OP_NEVER;
			sampler_info.minLod = info.min_lod;
			sampler_info.maxLod = info.max_lod;
			sampler_info.borderColor = GetVulkanBorderColor(info.border_color);
			sampler_info.unnormalizedCoordinates = VK_FALSE;

			VK_CHECK(vkCreateSampler(device.GetLogicalDevice(), &sampler_info, instance.GetAllocator(), sampler));
		}

		VulkanTexture2D::VulkanTexture2D() : VulkanTexture()
		{
			NO_OP;
		}

		VulkanTexture2D::~VulkanTexture2D()
		{
			NO_OP;
		}

		void VulkanTexture2D::Create(const VulkanInstance& instance, const VulkanDevice& device, const VulkanCommandPool& command_pool, VkFormat format, i32 width, i32 height)
		{
			m_format = format;
			m_channels = GetChannelCountFromVkFormat(m_format);
			m_mip_levels = 1;
			m_layer_count = 1;
			m_faces = 1;

			VkImageAspectFlags aspect = GetAspectMaskFromVkFormat(format);
			VkImageUsageFlags usage = GetUsageFlagsFromVkFormat(format);

			m_image.Create(instance, device, VK_IMAGE_TYPE_2D, m_format, { (u32)width, (u32)height, 1 }, m_mip_levels, m_layer_count * m_faces, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_TILING_OPTIMAL, usage, 0, 0);
			m_image.CreateImageView(instance, device, VK_IMAGE_VIEW_TYPE_2D, m_format, { aspect, 0, m_mip_levels, 0, m_layer_count });

			ImageSamplerCreationInfo info = {};
			info.max_lod = (f32)m_mip_levels;

			VulkanTexture::CreateSampler(instance, device, info, &m_sampler);
		}

		void VulkanTexture2D::Create(const VulkanInstance& instance, const VulkanDevice& device, const VulkanCommandPool& command_pool, VkFormat format, const void* data, i32 width, i32 height)
		{
			m_format = format;
			m_channels = GetChannelCountFromVkFormat(m_format);
			m_mip_levels = 1;
			m_layer_count = 1;
			m_faces = 1;

			VkImageSubresourceRange subresource_range = {};
			subresource_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			subresource_range.baseMipLevel = 0;
			subresource_range.levelCount = m_mip_levels;
			subresource_range.baseArrayLayer = 0;
			subresource_range.layerCount = m_layer_count;

			m_image.Create(instance, device, VK_IMAGE_TYPE_2D, m_format, { (u32)width, (u32)height, 1 }, m_mip_levels, m_layer_count * m_faces, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, 0, 0);
			m_image.CreateImageView(instance, device, VK_IMAGE_VIEW_TYPE_2D, m_format, subresource_range);
			
			VkDeviceSize image_size = m_image.GetWidth() * m_image.GetHeight() * m_image.GetDepth() * m_channels;

			VulkanBuffer stagging_buffer;
			stagging_buffer.Create(instance, device, image_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, false, 0, nullptr, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
			stagging_buffer.MapUnmap(device, image_size, 0, data);

			std::vector<VkBufferImageCopy> buffer_copy_regions;
			VkBufferImageCopy region = {};

			region.bufferOffset = 0;
			region.bufferRowLength = 0;
			region.bufferImageHeight = 0;

			region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			region.imageSubresource.mipLevel = 0;
			region.imageSubresource.baseArrayLayer = 0;
			region.imageSubresource.layerCount = 1;

			region.imageOffset = { 0, 0, 0 };
			region.imageExtent = { m_image.GetWidth(), m_image.GetHeight(), m_image.GetDepth() };

			buffer_copy_regions.push_back(region);

			VulkanImage::Transition(device, command_pool, m_image.GetHandle(), subresource_range, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
			stagging_buffer.CopyToImage(device, command_pool, m_image.GetHandle(), buffer_copy_regions);
			VulkanImage::Transition(device, command_pool, m_image.GetHandle(), subresource_range, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

			stagging_buffer.Destroy(instance, device);

			ImageSamplerCreationInfo info = {};
			info.max_lod = (f32)m_mip_levels;;

			VulkanTexture::CreateSampler(instance, device, info, &m_sampler);
		}

		void VulkanTexture2D::Create(const VulkanInstance& instance, const VulkanDevice& device, const VulkanCommandPool& command_pool, VkFormat format, const char* path)
		{
			ktxResult result;
			ktxTexture* ktx_texture;

			result = ktxTexture_CreateFromNamedFile(path, KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &ktx_texture);
			HE_ASSERT(result == KTX_SUCCESS, "Failed to load ktx texture image!");

			u32 width = ktx_texture->baseWidth;
			u32 height = ktx_texture->baseHeight;
			u32 depth = ktx_texture->baseDepth;
			m_format = format;
			m_channels = GetChannelCountFromVkFormat(m_format);
			m_mip_levels = ktx_texture->numLevels;
			m_layer_count = ktx_texture->numLayers;
			m_faces = ktx_texture->numFaces;

			VkImageSubresourceRange subresource_range = {};
			subresource_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			subresource_range.baseMipLevel = 0;
			subresource_range.levelCount = m_mip_levels;
			subresource_range.baseArrayLayer = 0;
			subresource_range.layerCount = m_layer_count;

			m_image.Create(instance, device, VK_IMAGE_TYPE_2D, m_format, { width, height, depth }, m_mip_levels, m_layer_count * m_faces, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, 0, 0);
			m_image.CreateImageView(instance, device, m_layer_count > 1 ? VK_IMAGE_VIEW_TYPE_2D_ARRAY : VK_IMAGE_VIEW_TYPE_2D, m_format, subresource_range);

			ktx_uint8_t* ktx_data = ktxTexture_GetData(ktx_texture);
			VkDeviceSize image_size = ktxTexture_GetDataSize(ktx_texture);

			VulkanBuffer stagging_buffer;
			stagging_buffer.Create(instance, device, image_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, false, 0, nullptr, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
			stagging_buffer.MapUnmap(device, image_size, 0, ktx_data);

			std::vector<VkBufferImageCopy> buffer_copy_regions;
			u32 offset = 0;

			for (u32 level = 0; level < m_mip_levels; level++)
			{
				for (u32 layer = 0; layer < m_layer_count; layer++)
				{
					ktx_size_t offset;
					ktx_error_code_e ret = ktxTexture_GetImageOffset(ktx_texture, level, layer, 0, &offset);
					HE_ASSERT(ret == KTX_SUCCESS, "Failed to get image offset!");

					VkBufferImageCopy buffer_copy_region = {};
					buffer_copy_region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
					buffer_copy_region.imageSubresource.mipLevel = level;
					buffer_copy_region.imageSubresource.baseArrayLayer = layer;
					buffer_copy_region.imageSubresource.layerCount = 1;
					buffer_copy_region.imageExtent.width = ktx_texture->baseWidth >> level;
					buffer_copy_region.imageExtent.height = ktx_texture->baseHeight >> level;
					buffer_copy_region.imageExtent.depth = 1;
					buffer_copy_region.bufferOffset = offset;
					buffer_copy_regions.push_back(buffer_copy_region);
				}
			}

			VulkanImage::Transition(device, command_pool, m_image.GetHandle(), subresource_range, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
			stagging_buffer.CopyToImage(device, command_pool, m_image.GetHandle(), buffer_copy_regions);
			VulkanImage::Transition(device, command_pool, m_image.GetHandle(), subresource_range, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

			stagging_buffer.Destroy(instance, device);
			ktxTexture_Destroy(ktx_texture);

			ImageSamplerCreationInfo info = {};
			info.max_lod = (f32)m_mip_levels;

			VulkanTexture::CreateSampler(instance, device, info, &m_sampler);
		}

		void VulkanTexture2D::LoadSTBI(const VulkanInstance& instance, const VulkanDevice& device, const VulkanCommandPool& command_pool, VkFormat format, const char* path)
		{
			i32 width, height, channels;
			stbi_uc* pixels = stbi_load(path, &width, &height, &channels, STBI_rgb_alpha);
			HE_ASSERT(pixels, "Failed to load texture image!");

			m_format = format;
			m_channels = GetChannelCountFromVkFormat(m_format);
			m_mip_levels = 1;
			m_layer_count = 1;
			m_faces = 1;

			VkImageSubresourceRange subresource_range = {};
			subresource_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			subresource_range.baseMipLevel = 0;
			subresource_range.levelCount = m_mip_levels;
			subresource_range.baseArrayLayer = 0;
			subresource_range.layerCount = m_layer_count;

			m_image.Create(instance, device, VK_IMAGE_TYPE_2D, m_format, { (u32)width, (u32)height, 1 }, m_mip_levels, m_layer_count * m_faces, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, 0, 0);
			m_image.CreateImageView(instance, device, VK_IMAGE_VIEW_TYPE_2D, m_format, subresource_range);

			VkDeviceSize image_size = width * height * m_channels;

			VulkanBuffer stagging_buffer;
			stagging_buffer.Create(instance, device, image_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, false, 0, nullptr, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
			stagging_buffer.MapUnmap(device, image_size, 0, pixels);

			stbi_image_free(pixels);

			std::vector<VkBufferImageCopy> buffer_copy_regions;
			VkBufferImageCopy region = {};

			region.bufferOffset = 0;
			region.bufferRowLength = 0;
			region.bufferImageHeight = 0;

			region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			region.imageSubresource.mipLevel = 0;
			region.imageSubresource.baseArrayLayer = 0;
			region.imageSubresource.layerCount = 1;
			region.imageOffset = { 0, 0, 0 };
			region.imageExtent = { m_image.GetWidth(), m_image.GetHeight(), m_image.GetDepth() };

			buffer_copy_regions.push_back(region);

			VulkanImage::Transition(device, command_pool, m_image.GetHandle(), subresource_range, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
			stagging_buffer.CopyToImage(device, command_pool, m_image.GetHandle(), buffer_copy_regions);
			VulkanImage::Transition(device, command_pool, m_image.GetHandle(), subresource_range, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

			stagging_buffer.Destroy(instance, device);

			ImageSamplerCreationInfo info = {};
			info.max_lod = (f32)m_mip_levels;

			VulkanTexture::CreateSampler(instance, device, info, &m_sampler);
		}

		VulkanTexture3D::VulkanTexture3D() : VulkanTexture()
		{
			NO_OP;
		}

		VulkanTexture3D::~VulkanTexture3D()
		{
			NO_OP;
		}

		void VulkanTexture3D::Create(const VulkanInstance& instance, const VulkanDevice& device, const VulkanCommandPool& command_pool, VkFormat format, i32 width, i32 height, i32 depth)
		{
			m_mip_levels = 1;
			m_format = format;
			m_channels = GetChannelCountFromVkFormat(m_format);
			m_layer_count = 1;
			m_faces = 1;

			m_image.Create(instance, device, VK_IMAGE_TYPE_3D, m_format, { (u32)width, (u32)height, (u32)depth }, m_mip_levels, m_layer_count * m_faces, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, 0, 0);
			m_image.CreateImageView(instance, device, VK_IMAGE_VIEW_TYPE_3D, m_format, { VK_IMAGE_ASPECT_COLOR_BIT, 0, m_mip_levels, 0, m_layer_count });

			ImageSamplerCreationInfo info = {};
			info.max_lod = (f32)m_mip_levels;
			
			VulkanTexture::CreateSampler(instance, device, info, &m_sampler);
		}

		void VulkanTexture3D::Create(const VulkanInstance& instance, const VulkanDevice& device, const VulkanCommandPool& command_pool, VkFormat format, const void* data, i32 width, i32 height, i32 depth)
		{
			m_format = format;
			m_channels = GetChannelCountFromVkFormat(m_format);
			m_mip_levels = 1;
			m_layer_count = 1;
			m_faces = 1;

			VkImageSubresourceRange subresource_range = {};
			subresource_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			subresource_range.baseMipLevel = 0;
			subresource_range.levelCount = m_mip_levels;
			subresource_range.baseArrayLayer = 0;
			subresource_range.layerCount = m_layer_count;

			m_image.Create(instance, device, VK_IMAGE_TYPE_3D, m_format, { (u32)width, (u32)height, (u32)depth }, m_mip_levels, m_layer_count * m_faces, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, 0, 0);
			m_image.CreateImageView(instance, device, VK_IMAGE_VIEW_TYPE_3D, m_format, subresource_range);

			u32 max_image_size = device.GetProperties().limits.maxImageDimension3D;
			HE_ASSERT(width <= (i32)max_image_size && height <= (i32)max_image_size && depth <= (i32)max_image_size, "Image size exceeds maximum image size!");

			VkDeviceSize image_size = width * height * depth;

			VulkanBuffer stagging_buffer;
			stagging_buffer.Create(instance, device, image_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, false, 0, nullptr, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
			stagging_buffer.MapUnmap(device, image_size, 0, data);

			std::vector<VkBufferImageCopy> buffer_copy_regions;
			VkBufferImageCopy region = {};
			region.bufferOffset = 0;
			region.bufferRowLength = 0;
			region.bufferImageHeight = 0;

			region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			region.imageSubresource.mipLevel = 0;
			region.imageSubresource.baseArrayLayer = 0;
			region.imageSubresource.layerCount = 1;

			region.imageOffset = { 0, 0, 0 };
			region.imageExtent = { (u32)width, (u32)height, (u32)depth };

			buffer_copy_regions.push_back(region);

			VulkanImage::Transition(device, command_pool, m_image.GetHandle(), subresource_range, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
			stagging_buffer.CopyToImage(device, command_pool, m_image.GetHandle(), buffer_copy_regions);
			VulkanImage::Transition(device, command_pool, m_image.GetHandle(), subresource_range, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

			stagging_buffer.Destroy(instance, device);

			ImageSamplerCreationInfo info = {};
			info.max_lod = (f32)m_mip_levels;

			VulkanTexture::CreateSampler(instance, device, info, &m_sampler);
		}

		VulkanTextureCubemap::VulkanTextureCubemap() : VulkanTexture()
		{
			NO_OP;
		}

		VulkanTextureCubemap::~VulkanTextureCubemap()
		{
			NO_OP;
		}

		void VulkanTextureCubemap::Create(const VulkanInstance& instance, const VulkanDevice& device, const VulkanCommandPool& command_pool, VkFormat format, const char* path)
		{
			ktxResult result;
			ktxTexture* ktx_texture;

			result = ktxTexture_CreateFromNamedFile(path, KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &ktx_texture);
			HE_ASSERT(result == KTX_SUCCESS, "Failed to load ktx texture image!");

			u32 width = ktx_texture->baseWidth;
			u32 height = ktx_texture->baseHeight;
			u32 depth = ktx_texture->baseDepth;
			m_format = format;
			m_channels = GetChannelCountFromVkFormat(m_format);
			m_mip_levels = ktx_texture->numLevels;
			m_layer_count = 6;
			m_faces = ktx_texture->numFaces;

			VkImageSubresourceRange subresource_range = {};
			subresource_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			subresource_range.baseMipLevel = 0;
			subresource_range.levelCount = m_mip_levels;
			subresource_range.baseArrayLayer = 0;
			subresource_range.layerCount = m_layer_count;

			m_image.Create(instance, device, VK_IMAGE_TYPE_2D, m_format, { width, height, depth }, m_mip_levels, m_layer_count * m_faces, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT, 0);
			m_image.CreateImageView(instance, device, VK_IMAGE_VIEW_TYPE_CUBE, m_format, subresource_range);

			ktx_uint8_t* ktx_data = ktxTexture_GetData(ktx_texture);
			VkDeviceSize image_size = ktxTexture_GetDataSize(ktx_texture);

			VulkanBuffer stagging_buffer;
			stagging_buffer.Create(instance, device, image_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, false, 0, nullptr, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
			stagging_buffer.MapUnmap(device, image_size, 0, ktx_data);

			std::vector<VkBufferImageCopy> buffer_copy_regions;
			u32 offset = 0;

			for (u32 level = 0; level < m_mip_levels; level++)
			{
				for (u32 layer = 0; layer < m_layer_count; layer++)
				{
					ktx_size_t offset;
					ktx_error_code_e ret = ktxTexture_GetImageOffset(ktx_texture, level, 0, layer, &offset);
					HE_ASSERT(ret == KTX_SUCCESS, "Failed to get image offset!");

					VkBufferImageCopy buffer_copy_region = {};
					buffer_copy_region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
					buffer_copy_region.imageSubresource.mipLevel = level;
					buffer_copy_region.imageSubresource.baseArrayLayer = layer;
					buffer_copy_region.imageSubresource.layerCount = 1;
					buffer_copy_region.imageExtent.width = ktx_texture->baseWidth >> level;
					buffer_copy_region.imageExtent.height = ktx_texture->baseHeight >> level;
					buffer_copy_region.imageExtent.depth = 1;
					buffer_copy_region.bufferOffset = offset;
					buffer_copy_regions.push_back(buffer_copy_region);
				}
			}

			VulkanImage::Transition(device, command_pool, m_image.GetHandle(), subresource_range, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
			stagging_buffer.CopyToImage(device, command_pool, m_image.GetHandle(), buffer_copy_regions);
			VulkanImage::Transition(device, command_pool, m_image.GetHandle(), subresource_range, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

			stagging_buffer.Destroy(instance, device);
			ktxTexture_Destroy(ktx_texture);

			ImageSamplerCreationInfo info = {};
			info.max_lod = (f32)m_mip_levels;

			VulkanTexture::CreateSampler(instance, device, info, &m_sampler);
		}

		void VulkanTextureCubemap::CreateArray(const VulkanInstance& instance, const VulkanDevice& device, const VulkanCommandPool& command_pool, VkFormat format, const char* path)
		{
			ktxResult result;
			ktxTexture* ktx_texture;

			result = ktxTexture_CreateFromNamedFile(path, KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &ktx_texture);
			HE_ASSERT(result == KTX_SUCCESS, "Failed to load ktx texture image!");

			u32 width = ktx_texture->baseWidth;
			u32 height = ktx_texture->baseHeight;
			u32 depth = ktx_texture->baseDepth;
			m_format = format;
			m_channels = GetChannelCountFromVkFormat(m_format);
			m_mip_levels = ktx_texture->numLevels;
			m_layer_count = ktx_texture->numLayers;
			m_faces = ktx_texture->numFaces;

			VkImageSubresourceRange subresource_range = {};
			subresource_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			subresource_range.baseMipLevel = 0;
			subresource_range.levelCount = m_mip_levels;
			subresource_range.baseArrayLayer = 0;
			subresource_range.layerCount = m_layer_count * m_faces;

			m_image.Create(instance, device, VK_IMAGE_TYPE_2D, m_format, { width, height, depth }, m_mip_levels, m_layer_count * m_faces, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT, 0);
			m_image.CreateImageView(instance, device, VK_IMAGE_VIEW_TYPE_CUBE_ARRAY, m_format, subresource_range);

			ktx_uint8_t* ktx_data = ktxTexture_GetData(ktx_texture);
			VkDeviceSize image_size = ktxTexture_GetDataSize(ktx_texture);

			VulkanBuffer stagging_buffer;
			stagging_buffer.Create(instance, device, image_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, false, 0, nullptr, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
			stagging_buffer.MapUnmap(device, image_size, 0, ktx_data);

			std::vector<VkBufferImageCopy> buffer_copy_regions;
			u32 offset = 0;

			for (u32 face = 0; face < m_faces; face++)
			{
				for (u32 layer = 0; layer < m_layer_count; layer++)
				{
					for (u32 level = 0; level < m_mip_levels; level++)
					{
						ktx_size_t offset;
						ktx_error_code_e ret = ktxTexture_GetImageOffset(ktx_texture, level, layer, face, &offset);
						HE_ASSERT(ret == KTX_SUCCESS, "Failed to get image offset!");

						VkBufferImageCopy buffer_copy_region = {};
						buffer_copy_region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
						buffer_copy_region.imageSubresource.mipLevel = level;
						buffer_copy_region.imageSubresource.baseArrayLayer = layer * 6 + face;
						buffer_copy_region.imageSubresource.layerCount = 1;
						buffer_copy_region.imageExtent.width = ktx_texture->baseWidth >> level;
						buffer_copy_region.imageExtent.height = ktx_texture->baseHeight >> level;
						buffer_copy_region.imageExtent.depth = 1;
						buffer_copy_region.bufferOffset = offset;
						buffer_copy_regions.push_back(buffer_copy_region);
					}
				}
			}

			VulkanImage::Transition(device, command_pool, m_image.GetHandle(), subresource_range, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
			stagging_buffer.CopyToImage(device, command_pool, m_image.GetHandle(), buffer_copy_regions);
			VulkanImage::Transition(device, command_pool, m_image.GetHandle(), subresource_range, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

			stagging_buffer.Destroy(instance, device);
			ktxTexture_Destroy(ktx_texture);

			ImageSamplerCreationInfo info = {};
			info.max_lod = (f32)m_mip_levels;

			VulkanTexture::CreateSampler(instance, device, info, &m_sampler);
		}

		VkImageAspectFlags GetAspectMaskFromVkFormat(VkFormat format)
		{
			VkImageAspectFlags aspect_mask = VK_IMAGE_ASPECT_NONE;
			if (format < VK_FORMAT_D16_UNORM)
			{
				aspect_mask = VK_IMAGE_ASPECT_COLOR_BIT;
			}
			else if (format == VK_FORMAT_D16_UNORM || format == VK_FORMAT_D32_SFLOAT)
			{
				aspect_mask = VK_IMAGE_ASPECT_DEPTH_BIT;
			}
			else if (format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT || format == VK_FORMAT_D16_UNORM_S8_UINT)
			{
				aspect_mask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
			}
			else if (format == VK_FORMAT_S8_UINT)
			{
				aspect_mask = VK_IMAGE_ASPECT_STENCIL_BIT;
			}

			return aspect_mask;
		}

		VkImageUsageFlags GetUsageFlagsFromVkFormat(VkFormat format)
		{
			VkImageUsageFlags usage = 0;
			if (format < VK_FORMAT_D16_UNORM)
			{
				usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
			}
			else
			{
				usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
			}

			return usage;
		}

} // namespace graphics

} // namespace hellengine