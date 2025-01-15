#pragma once

// Internal
#include "vulkan_core.h"

namespace hellengine
{

	namespace graphics
	{
		typedef VulkanBuffer* Buffer;
		typedef VulkanCommandBuffer* CommandBuffer;
		typedef VulkanDevice* Device;
		typedef VulkanDescriptorSet* DescriptorSet;
		typedef VulkanInstance* Instance;
		typedef VulkanPipeline* Pipeline;
		typedef VulkanTexture* Texture;
		typedef VulkanUniformBuffer* UniformBuffer;

	} // namespace graphics

} // namespace hellengine