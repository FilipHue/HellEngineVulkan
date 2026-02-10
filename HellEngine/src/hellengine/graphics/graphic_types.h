#pragma once

// Internal
#include "vulkan_core.h"

namespace hellengine
{

	namespace graphics
	{
		using Buffer = VulkanBuffer;
		using CommandBuffer = VulkanCommandBuffer;
		using DescriptorSet = VulkanDescriptorSet;
		using Device = VulkanDevice;
		using Instance = VulkanInstance;
		using Pipeline = VulkanPipeline;
		using RenderPass = VulkanRenderPass;
		using Texture2D = VulkanTexture2D;
		using Texture3D = VulkanTexture3D;
		using TextureCubemap = VulkanTextureCubemap;
		using UniformBuffer = VulkanUniformBuffer;
		using StorageBuffer = VulkanStorageBuffer;

	} // namespace graphics

} // namespace hellengine