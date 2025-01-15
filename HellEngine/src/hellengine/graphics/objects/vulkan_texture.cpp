#include "hepch.h"
#include "vulkan_texture.h"

// Internal
#include "vulkan_buffer.h"
#include "vulkan_command_buffer.h"
#include "vulkan_device.h"
#include "vulkan_instance.h"
#include "vulkan_image.h"

// External
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

namespace hellengine
{

	namespace graphics
	{

		VulkanTexture::VulkanTexture()
		{
			m_handle = VK_NULL_HANDLE;
			m_image_memory = VK_NULL_HANDLE;
			m_image_view = VK_NULL_HANDLE;
			m_sampler = VK_NULL_HANDLE;

			m_width = 0;
			m_height = 0;
			m_channels = 0;
		}

		VulkanTexture::~VulkanTexture()
		{
		}

		void VulkanTexture::Create(const VulkanInstance& instance, const VulkanDevice& device, const VulkanCommandPool& command_pool, const char* path)
		{
			stbi_uc* pixels = stbi_load(path, &m_width, &m_height, &m_channels, STBI_rgb_alpha);
			VkDeviceSize image_size = m_width * m_height * 4;

			HE_ASSERT(pixels, "Failed to load texture image!");

			VulkanBuffer stagging_buffer;
			stagging_buffer.Create(instance, device, image_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
			stagging_buffer.MapUnmap(device, image_size, 0, pixels);

			stbi_image_free(pixels);

			CreateImage(instance, device);
			AllocateDeviceMemory(instance, device);

			VulkanImage::Transition(device, command_pool, m_handle, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
			stagging_buffer.CopyToImage(device, command_pool, m_handle, m_width, m_height);
			VulkanImage::Transition(device, command_pool, m_handle, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

			stagging_buffer.Destroy(instance, device);

			CreateImageView(instance, device, m_handle, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, m_image_view);
			CreateSampler(instance, device);
		}

		void VulkanTexture::Destroy(const VulkanInstance& instance, const VulkanDevice& device) const
		{
			vkDestroySampler(device.GetLogicalDevice(), m_sampler, instance.GetAllocator());
			vkDestroyImageView(device.GetLogicalDevice(), m_image_view, instance.GetAllocator());
			vkDestroyImage(device.GetLogicalDevice(), m_handle, instance.GetAllocator());
			vkFreeMemory(device.GetLogicalDevice(), m_image_memory, instance.GetAllocator());
		}

		void VulkanTexture::CreateImage(const VulkanInstance& instance, const VulkanDevice& device)
		{
			VkImageCreateInfo image_info = {};
			image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			image_info.pNext = VK_NULL_HANDLE;
			image_info.flags = 0;
			image_info.imageType = VK_IMAGE_TYPE_2D;
			image_info.format = VK_FORMAT_R8G8B8A8_SRGB;
			image_info.extent = { (u32)m_width, (u32)m_height, 1 };
			image_info.mipLevels = 1;
			image_info.arrayLayers = 1;
			image_info.samples = VK_SAMPLE_COUNT_1_BIT;
			image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
			image_info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
			image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			image_info.queueFamilyIndexCount = 0;
			image_info.pQueueFamilyIndices = VK_NULL_HANDLE;
			image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

			VK_CHECK(vkCreateImage(device.GetLogicalDevice(), &image_info, instance.GetAllocator(), &m_handle));
		}

		void VulkanTexture::AllocateDeviceMemory(const VulkanInstance& instance, const VulkanDevice& device)
		{
			VkMemoryRequirements memory_requirements;
			vkGetImageMemoryRequirements(device.GetLogicalDevice(), m_handle, &memory_requirements);

			VkMemoryAllocateInfo allocate_info = {};
			allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			allocate_info.pNext = VK_NULL_HANDLE;
			allocate_info.allocationSize = memory_requirements.size;
			allocate_info.memoryTypeIndex = VulkanDevice::FindMemoryType(device, memory_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

			VK_CHECK(vkAllocateMemory(device.GetLogicalDevice(), &allocate_info, instance.GetAllocator(), &m_image_memory));
			VK_CHECK(vkBindImageMemory(device.GetLogicalDevice(), m_handle, m_image_memory, 0));
		}

		void VulkanTexture::CreateSampler(const VulkanInstance& instance, const VulkanDevice& device)
		{
			VkSamplerCreateInfo sampler_info = {};
			sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
			sampler_info.pNext = VK_NULL_HANDLE;
			sampler_info.flags = 0;
			sampler_info.magFilter = VK_FILTER_LINEAR;
			sampler_info.minFilter = VK_FILTER_LINEAR;
			sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			sampler_info.anisotropyEnable = VK_TRUE;

			VkPhysicalDeviceProperties properties = {};
			vkGetPhysicalDeviceProperties(device.GetPhysicalDevice(), &properties);
			sampler_info.maxAnisotropy = properties.limits.maxSamplerAnisotropy;

			sampler_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
			sampler_info.unnormalizedCoordinates = VK_FALSE;
			sampler_info.compareEnable = VK_FALSE;
			sampler_info.compareOp = VK_COMPARE_OP_ALWAYS;
			sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
			sampler_info.mipLodBias = 0.0f;
			sampler_info.minLod = 0.0f;
			sampler_info.maxLod = 0.0f;

			VK_CHECK(vkCreateSampler(device.GetLogicalDevice(), &sampler_info, instance.GetAllocator(), &m_sampler));
		}

	} // namespace graphics

} // namespace hellengine