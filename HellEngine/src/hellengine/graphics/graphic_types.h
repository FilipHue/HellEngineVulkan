#pragma once

// Internal
#include "vulkan_core.h"

namespace hellengine
{

	namespace graphics
	{
		typedef VulkanBuffer* Buffer;
		typedef VulkanCommandBuffer* CommandBuffer;
		typedef VulkanDescriptorSet* DescriptorSet;
		typedef VulkanDevice* Device;
		typedef VulkanInstance* Instance;
		using Pipeline = VulkanPipeline;
		typedef VulkanRenderPass* RenderPass;
		typedef VulkanTexture2D* Texture2D;
		typedef VulkanTexture3D* Texture3D;
		typedef VulkanTextureCubemap* TextureCubemap;
		typedef VulkanUniformBuffer* UniformBuffer;
		typedef VulkanStorageBuffer* StorageBuffer;

	} // namespace graphics

} // namespace hellengine