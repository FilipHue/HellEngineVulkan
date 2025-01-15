#pragma once

// Internal
#include <hellengine/graphics/core.h>
#include <hellengine/graphics/objects/types.h>

namespace hellengine
{

	namespace graphics
	{

#define MAX_FRAMES_IN_FLIGHT 3

		// Forward declarations
		class VulkanBuffer;
		class VulkanCommandBuffer;
		class VulkanCommandPool;
		class VulkanDescriptorPool;
		class VulkanDescriptorSet;
		class VulkanDevice;
		class VulkanFence;
		class VulkanImage;
		class VulkanInstance;
		class VulkanPipeline;
		class VulkanShader;
		class VulkanSurface;
		class VulkanSwapchain;
		class VulkanSemaphore;
		class VulkanTexture;

		INLINE VkShaderStageFlagBits ToVulkanShaderStageFlags(ShaderStage stage)
		{
			VkShaderStageFlagBits stage_vk = (VkShaderStageFlagBits)0;

			if (stage & ShaderStage_Vertex)
			{
				stage_vk = (VkShaderStageFlagBits)(stage_vk | VK_SHADER_STAGE_VERTEX_BIT);
			}

			if (stage & ShaderStage_Fragment)
			{
				stage_vk = (VkShaderStageFlagBits)(stage_vk | VK_SHADER_STAGE_FRAGMENT_BIT);
			}

			if (stage & ShaderStage_Compute)
			{
				stage_vk = (VkShaderStageFlagBits)(stage_vk | VK_SHADER_STAGE_COMPUTE_BIT);
			}

			if (stage & ShaderStage_Geometry)
			{
				stage_vk = (VkShaderStageFlagBits)(stage_vk | VK_SHADER_STAGE_GEOMETRY_BIT);
			}

			return stage_vk;
		}

		INLINE VkShaderStageFlagBits ShaderTypeToVulkan(ShaderType type)
		{
			switch (type)
			{
			case ShaderType_Vertex:
				return VK_SHADER_STAGE_VERTEX_BIT;
			case ShaderType_Geometry:
				return VK_SHADER_STAGE_GEOMETRY_BIT;
			case ShaderType_Fragment:
				return VK_SHADER_STAGE_FRAGMENT_BIT;
			case ShaderType_Compute:
				return VK_SHADER_STAGE_COMPUTE_BIT;
			default:
				return VK_SHADER_STAGE_VERTEX_BIT;
			}
		}

		INLINE VkDescriptorType ToVulkanDescriptorType(DescriptorType type)
		{
			switch (type)
			{
			case DescriptorType_UniformBuffer:
				return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			case DescriptorType_StorageBuffer:
				return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			case DescriptorType_UniformTexelBuffer:
				return VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
			case DescriptorType_StorageTexelBuffer:
				return VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
			case DescriptorType_UniformBufferDynamic:
				return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
			case DescriptorType_StorageBufferDynamic:
				return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
			case DescriptorType_InputAttachment:
				return VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
			case DescriptorType_StorageImage:
				return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
			case DescriptorType_SampledImage:
				return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
			case DescriptorType_CombinedImageSampler:
				return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			case DescriptorType_Sampler:
				return VK_DESCRIPTOR_TYPE_SAMPLER;
			default:
				return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			}
		}

		INLINE VkPipelineBindPoint GetVulkanBindPoint(PipelineType type)
		{
			switch (type)
			{
			case PipelineType_Graphics:
				return VK_PIPELINE_BIND_POINT_GRAPHICS;
			case PipelineType_Compute:
				return VK_PIPELINE_BIND_POINT_COMPUTE;
			default:
				return VK_PIPELINE_BIND_POINT_GRAPHICS;
			}
		}

		INLINE VkPrimitiveTopology GetVulkanTopology(PipelinePrimitiveTopology topo)
		{
			switch (topo)
			{
			case PipelinePrimitiveTopology_LineList:
				return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
			case PipelinePrimitiveTopology_TriangleList:
				return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
			default:
				return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
			}
		}

		INLINE VkCullModeFlags GetVulkanCullMode(PipelineCullMode cull_mode)
		{
			switch (cull_mode)
			{
			case PipelineCullMode_None:
				return VK_CULL_MODE_NONE;
			case PipelineCullMode_Front:
				return VK_CULL_MODE_FRONT_BIT;
			case PipelineCullMode_Back:
				return VK_CULL_MODE_BACK_BIT;
			default:
				return VK_CULL_MODE_BACK_BIT;
			}
		}

		INLINE VkPolygonMode GetVulkanPolygonMode(PipelinePolygonMode polygon_mode)
		{
			switch (polygon_mode)
			{
			case PipelinePolygonMode_Fill:
				return VK_POLYGON_MODE_FILL;
			case PipelinePolygonMode_Line:
				return VK_POLYGON_MODE_LINE;
			case PipelinePolygonMode_Point:
				return VK_POLYGON_MODE_POINT;
			default:
				return VK_POLYGON_MODE_FILL;
			}
		}

		INLINE VkFrontFace GetVulkanFrontFace(PipelineFrontFace front_face)
		{
			switch (front_face)
			{
			case PipelineFrontFace_Clockwise:
				return VK_FRONT_FACE_CLOCKWISE;
			case PipelineFrontFace_CounterClockwise:
				return VK_FRONT_FACE_COUNTER_CLOCKWISE;
			default:
				return VK_FRONT_FACE_COUNTER_CLOCKWISE;
			}
		}

		INLINE VkDynamicState GetVulkanDynamicState(PipelineDynamicState dynamic_state)
		{
			switch (dynamic_state)
			{
			case PipelineDynamicState_Viewport:
				return VK_DYNAMIC_STATE_VIEWPORT;
			case PipelineDynamicState_Scissor:
				return VK_DYNAMIC_STATE_SCISSOR;
			case PipelineDynamicState_LineWidth:
				return VK_DYNAMIC_STATE_LINE_WIDTH;
			case PipelineDynamicState_DepthBias:
				return VK_DYNAMIC_STATE_DEPTH_BIAS;
			case PipelineDynamicState_BlendConstants:
				return VK_DYNAMIC_STATE_BLEND_CONSTANTS;
			case PipelineDynamicState_DepthBounds:
				return VK_DYNAMIC_STATE_DEPTH_BOUNDS;
			case PipelineDynamicState_StencilCompareMask:
				return VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK;
			case PipelineDynamicState_StencilWriteMask:
				return VK_DYNAMIC_STATE_STENCIL_WRITE_MASK;
			case PipelineDynamicState_StencilReference:
				return VK_DYNAMIC_STATE_STENCIL_REFERENCE;
			default:
				return VK_DYNAMIC_STATE_VIEWPORT;
			}
		}

	} // namespace graphics

} // namespace hellengine