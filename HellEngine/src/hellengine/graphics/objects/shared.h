#pragma once

// Internal
#include <hellengine/graphics/graphics_core.h>
#include <hellengine/graphics/objects/types.h>

namespace hellengine
{

	using namespace core;
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
		class VulkanFramebuffer;
		class VulkanImage;
		class VulkanInstance;
		class VulkanPipeline;
		class VulkanRenderPass;
		class VulkanShader;
		class VulkanSurface;
		class VulkanSwapchain;
		class VulkanSemaphore;
		class VulkanTexture2D;

		/************\
		| RENDERPASS |
		\************/

		INLINE VkAttachmentLoadOp GetVulkanAttachmentLoadOp(AttachmentLoadOp load_op)
		{
			switch (load_op)
			{
			case AttachmentLoadOp_Load:
				return VK_ATTACHMENT_LOAD_OP_LOAD;
			case AttachmentLoadOp_Clear:
				return VK_ATTACHMENT_LOAD_OP_CLEAR;
			case AttachmentLoadOp_DontCare:
				return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			default:
				return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			}
		}

		INLINE VkAttachmentStoreOp GetVulkanAttachmentStoreOp(AttachmentStoreOp store_op)
		{
			switch (store_op)
			{
			case AttachmentStoreOp_Store:
				return VK_ATTACHMENT_STORE_OP_STORE;
			case AttachmentStoreOp_DontCare:
				return VK_ATTACHMENT_STORE_OP_DONT_CARE;
			default:
				return VK_ATTACHMENT_STORE_OP_DONT_CARE;
			}
		}

		/**************\
		| SHADER STAGE |
		\**************/

		INLINE ShaderStage operator|(ShaderStage lhs, ShaderStage rhs)
		{
			return static_cast<ShaderStage>(static_cast<u32>(lhs) | static_cast<u32>(rhs));
		}

		INLINE VkShaderStageFlagBits GetVulkanStageFlags(ShaderStage stage)
		{
			VkShaderStageFlagBits stage_vk = (VkShaderStageFlagBits)0;

			if (stage & ShaderStage_Vertex)
			{
				stage_vk = (VkShaderStageFlagBits)(stage_vk | VK_SHADER_STAGE_VERTEX_BIT);
			}

			if (stage & ShaderStage_TessControl)
			{
				stage_vk = (VkShaderStageFlagBits)(stage_vk | VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT);
			}

			if (stage & ShaderStage_TessEvaluation)
			{
				stage_vk = (VkShaderStageFlagBits)(stage_vk | VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT);
			}

			if (stage & ShaderStage_Fragment)
			{
				stage_vk = (VkShaderStageFlagBits)(stage_vk | VK_SHADER_STAGE_FRAGMENT_BIT);
			}

			if (stage & ShaderStage_Geometry)
			{
				stage_vk = (VkShaderStageFlagBits)(stage_vk | VK_SHADER_STAGE_GEOMETRY_BIT);
			}

			if (stage & ShaderStage_Compute)
			{
				stage_vk = (VkShaderStageFlagBits)(stage_vk | VK_SHADER_STAGE_COMPUTE_BIT);
			}

			return stage_vk;
		}

		INLINE VkShaderStageFlagBits GetVulkanShaderStage(ShaderType type)
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

		/*************\
		| DESCRIPTORS |
		\*************/

		INLINE VkDescriptorType GetVulkanDescriptorType(DescriptorType type)
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

		INLINE DescriptorBindingFlags operator|(DescriptorBindingFlags lhs, DescriptorBindingFlags rhs)
		{
			return static_cast<DescriptorBindingFlags>(static_cast<u32>(lhs) | static_cast<u32>(rhs));
		}

		INLINE VkDescriptorBindingFlags GetVulkanDescriptorSetBindingFlags(DescriptorBindingFlags flags)
		{
			VkDescriptorBindingFlags vk_flags = (VkDescriptorBindingFlags)0;

			if (flags & DescriptorBindingFlags_UpdateAfterBind)
			{
				vk_flags = (VkDescriptorBindingFlags)(vk_flags | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT);
			}

			if (flags & DescriptorBindingFlags_UpdateUnusedWhilePending)
			{
				vk_flags = (VkDescriptorBindingFlags)(vk_flags | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT);
			}

			if (flags & DescriptorBindingFlags_PartiallyBound)
			{
				vk_flags = (VkDescriptorBindingFlags)(vk_flags | VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT);
			}

			if (flags & DescriptorBindingFlags_VariableCount)
			{
				vk_flags = (VkDescriptorBindingFlags)(vk_flags | VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT);
			}

			return vk_flags;
		}

		INLINE DescriptorSetFlags operator|(DescriptorSetFlags lhs, DescriptorSetFlags rhs)
		{
			return static_cast<DescriptorSetFlags>(static_cast<u32>(lhs) | static_cast<u32>(rhs));
		}

		INLINE VkDescriptorSetLayoutCreateFlagBits GetVulkanDescriptorSetFlags(DescriptorSetFlags flags)
		{
			VkDescriptorSetLayoutCreateFlagBits vk_flags = (VkDescriptorSetLayoutCreateFlagBits)0;

			if (flags & DescriptorSetFlags_UpdateAfterBindPool)
			{
				vk_flags = (VkDescriptorSetLayoutCreateFlagBits)(vk_flags | VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT);
			}

			return vk_flags;
		}

		/**********\
		| PIPELINE |
		\**********/

		INLINE VkPipelineBindPoint GetVulkanBindPoint(PipelineType type)
		{
			switch (type)
			{
			case PipelineType_Graphics:
				return VK_PIPELINE_BIND_POINT_GRAPHICS;
			default:
				return VK_PIPELINE_BIND_POINT_GRAPHICS;
			}
		}

		INLINE VkPrimitiveTopology GetVulkanTopology(PipelinePrimitiveTopology topo)
		{
			switch (topo)
			{
			case PipelinePrimitiveTopology_PointList:
				return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
			case PipelinePrimitiveTopology_LineList:
				return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
			case PipelinePrimitiveTopology_TriangleList:
				return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
			case PipelinePrimitiveTopology_TriangleStrip:
				return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
			case PipelinePrimitiveTopology_TriangleFan:
				return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
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

		INLINE VkCompareOp GetVulkanCompareOp(PipelineDethStencilCompareOp compare_op)
		{
			switch (compare_op)
			{
			case PipelineDethStencilCompareOp_Never:
				return VK_COMPARE_OP_NEVER;
			case PipelineDethStencilCompareOp_Less:
				return VK_COMPARE_OP_LESS;
			case PipelineDethStencilCompareOp_Equal:
				return VK_COMPARE_OP_EQUAL;
			case PipelineDethStencilCompareOp_LessOrEqual:
				return VK_COMPARE_OP_LESS_OR_EQUAL;
			case PipelineDethStencilCompareOp_Greater:
				return VK_COMPARE_OP_GREATER;
			case PipelineDethStencilCompareOp_NotEqual:
				return VK_COMPARE_OP_NOT_EQUAL;
			case PipelineDethStencilCompareOp_GreaterOrEqual:
				return VK_COMPARE_OP_GREATER_OR_EQUAL;
			case PipelineDethStencilCompareOp_Always:
				return VK_COMPARE_OP_ALWAYS;
			default:
				return VK_COMPARE_OP_ALWAYS;
			}
		}

		/********\
		|  IMAGE |
		\********/

		INLINE VkFilter GetVulkanFilter(ImageFilterMode filter_mode)
		{
			switch (filter_mode)
			{
			case ImageFilterMode_Nearest:
				return VK_FILTER_NEAREST;
			case ImageFilterMode_Linear:
				return VK_FILTER_LINEAR;
			default:
				return VK_FILTER_LINEAR;
			}
		}

		INLINE VkSamplerAddressMode GetVulkanAddressMode(ImageAddressMode address_mode)
		{
			switch (address_mode)
			{
			case ImageAddressMode_Repeat:
				return VK_SAMPLER_ADDRESS_MODE_REPEAT;
			case ImageAddressMode_MirroredRepeat:
				return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
			case ImageAddressMode_ClampToEdge:
				return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			case ImageAddressMode_ClampToBorder:
				return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
			case ImageAddressMode_MirroredClampToEdge:
				return VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;
			default:
				return VK_SAMPLER_ADDRESS_MODE_REPEAT;
			}
		}

		INLINE VkCompareOp GetVulkanCompareOp(ImageCompareOp compare_op)
		{
			switch (compare_op)
			{
			case ImageCompareOp_Never:
				return VK_COMPARE_OP_NEVER;
			case ImageCompareOp_Less:
				return VK_COMPARE_OP_LESS;
			case ImageCompareOp_Equal:
				return VK_COMPARE_OP_EQUAL;
			case ImageCompareOp_LessOrEqual:
				return VK_COMPARE_OP_LESS_OR_EQUAL;
			case ImageCompareOp_Greater:
				return VK_COMPARE_OP_GREATER;
			case ImageCompareOp_NotEqual:
				return VK_COMPARE_OP_NOT_EQUAL;
			case ImageCompareOp_GreaterOrEqual:
				return VK_COMPARE_OP_GREATER_OR_EQUAL;
			case ImageCompareOp_Always:
				return VK_COMPARE_OP_ALWAYS;
			default:
				return VK_COMPARE_OP_ALWAYS;
			}
		}

		INLINE VkBorderColor GetVulkanBorderColor(ImageBorderColor border_color)
		{
			switch (border_color)
			{
			case ImageBorderColor_FloatTransparentBlack:
				return VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
			case ImageBorderColor_IntTransparentBlack:
				return VK_BORDER_COLOR_INT_TRANSPARENT_BLACK;
			case ImageBorderColor_FloatOpaqueBlack:
				return VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
			case ImageBorderColor_IntOpaqueBlack:
				return VK_BORDER_COLOR_INT_OPAQUE_BLACK;
			case ImageBorderColor_FloatOpaqueWhite:
				return VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
			case ImageBorderColor_IntOpaqueWhite:
				return VK_BORDER_COLOR_INT_OPAQUE_WHITE;
			default:
				return VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
			}
		}

		INLINE VkFormat GetVulkanFormat(ImageFormat format)
		{
			switch(format)
			{
				case ImageFormat_UNDEFINED:						return VK_FORMAT_UNDEFINED;
				case ImageFormat_R4G4_UNORM_PACK8:				return VK_FORMAT_R4G4_UNORM_PACK8;
				case ImageFormat_R4G4B4A4_UNORM_PACK16:			return VK_FORMAT_R4G4B4A4_UNORM_PACK16;
				case ImageFormat_B4G4R4A4_UNORM_PACK16:			return VK_FORMAT_B4G4R4A4_UNORM_PACK16;
				case ImageFormat_R5G6B5_UNORM_PACK16:			return VK_FORMAT_R5G6B5_UNORM_PACK16;
				case ImageFormat_B5G6R5_UNORM_PACK16:			return VK_FORMAT_B5G6R5_UNORM_PACK16;
				case ImageFormat_R5G5B5A1_UNORM_PACK16:			return VK_FORMAT_R5G5B5A1_UNORM_PACK16;
				case ImageFormat_B5G5R5A1_UNORM_PACK16:			return VK_FORMAT_B5G5R5A1_UNORM_PACK16;
				case ImageFormat_A1R5G5B5_UNORM_PACK16:			return VK_FORMAT_A1R5G5B5_UNORM_PACK16;
				case ImageFormat_R8_UNORM:						return VK_FORMAT_R8_UNORM;
				case ImageFormat_R8_SNORM:						return VK_FORMAT_R8_SNORM;
				case ImageFormat_R8_USCALED:					return VK_FORMAT_R8_USCALED;
				case ImageFormat_R8_SSCALED:					return VK_FORMAT_R8_SSCALED;
				case ImageFormat_R8_UINT:						return VK_FORMAT_R8_UINT;
				case ImageFormat_R8_SINT:						return VK_FORMAT_R8_SINT;
				case ImageFormat_R8_SRGB:						return VK_FORMAT_R8_SRGB;
				case ImageFormat_R8G8_UNORM:					return VK_FORMAT_R8G8_UNORM;
				case ImageFormat_R8G8_SNORM:					return VK_FORMAT_R8G8_SNORM;
				case ImageFormat_R8G8_USCALED:					return VK_FORMAT_R8G8_USCALED;
				case ImageFormat_R8G8_SSCALED:					return VK_FORMAT_R8G8_SSCALED;
				case ImageFormat_R8G8_UINT:						return VK_FORMAT_R8G8_UINT;
				case ImageFormat_R8G8_SINT:						return VK_FORMAT_R8G8_SINT;
				case ImageFormat_R8G8_SRGB:						return VK_FORMAT_R8G8_SRGB;
				case ImageFormat_R8G8B8_UNORM:					return VK_FORMAT_R8G8B8_UNORM;
				case ImageFormat_R8G8B8_SNORM:					return VK_FORMAT_R8G8B8_SNORM;
				case ImageFormat_R8G8B8_USCALED:				return VK_FORMAT_R8G8B8_USCALED;
				case ImageFormat_R8G8B8_SSCALED:				return VK_FORMAT_R8G8B8_SSCALED;
				case ImageFormat_R8G8B8_UINT:					return VK_FORMAT_R8G8B8_UINT;
				case ImageFormat_R8G8B8_SINT:					return VK_FORMAT_R8G8B8_SINT;
				case ImageFormat_R8G8B8_SRGB:					return VK_FORMAT_R8G8B8_SRGB;
				case ImageFormat_B8G8R8_UNORM:					return VK_FORMAT_B8G8R8_UNORM;
				case ImageFormat_B8G8R8_SNORM:					return VK_FORMAT_B8G8R8_SNORM;
				case ImageFormat_B8G8R8_USCALED:				return VK_FORMAT_B8G8R8_USCALED;
				case ImageFormat_B8G8R8_SSCALED:				return VK_FORMAT_B8G8R8_SSCALED;
				case ImageFormat_B8G8R8_UINT:					return VK_FORMAT_B8G8R8_UINT;
				case ImageFormat_B8G8R8_SINT:					return VK_FORMAT_B8G8R8_SINT;
				case ImageFormat_B8G8R8_SRGB:					return VK_FORMAT_B8G8R8_SRGB;
				case ImageFormat_R8G8B8A8_UNORM:				return VK_FORMAT_R8G8B8A8_UNORM;
				case ImageFormat_R8G8B8A8_SNORM:				return VK_FORMAT_R8G8B8A8_SNORM;
				case ImageFormat_R8G8B8A8_USCALED:				return VK_FORMAT_R8G8B8A8_USCALED;
				case ImageFormat_R8G8B8A8_SSCALED:				return VK_FORMAT_R8G8B8A8_SSCALED;
				case ImageFormat_R8G8B8A8_UINT:					return VK_FORMAT_R8G8B8A8_UINT;
				case ImageFormat_R8G8B8A8_SINT:					return VK_FORMAT_R8G8B8A8_SINT;
				case ImageFormat_R8G8B8A8_SRGB:					return VK_FORMAT_R8G8B8A8_SRGB;
				case ImageFormat_B8G8R8A8_UNORM:				return VK_FORMAT_B8G8R8A8_UNORM;
				case ImageFormat_B8G8R8A8_SNORM:				return VK_FORMAT_B8G8R8A8_SNORM;
				case ImageFormat_B8G8R8A8_USCALED:				return VK_FORMAT_B8G8R8A8_USCALED;
				case ImageFormat_B8G8R8A8_SSCALED:				return VK_FORMAT_B8G8R8A8_SSCALED;
				case ImageFormat_B8G8R8A8_UINT:					return VK_FORMAT_B8G8R8A8_UINT;
				case ImageFormat_B8G8R8A8_SINT:					return VK_FORMAT_B8G8R8A8_SINT;
				case ImageFormat_B8G8R8A8_SRGB:					return VK_FORMAT_B8G8R8A8_SRGB;
				case ImageFormat_A8B8G8R8_UNORM_PACK32:			return VK_FORMAT_A8B8G8R8_UNORM_PACK32;
				case ImageFormat_A8B8G8R8_SNORM_PACK32:			return VK_FORMAT_A8B8G8R8_SNORM_PACK32;
				case ImageFormat_A8B8G8R8_USCALED_PACK32:		return VK_FORMAT_A8B8G8R8_USCALED_PACK32;
				case ImageFormat_A8B8G8R8_SSCALED_PACK32:		return VK_FORMAT_A8B8G8R8_SSCALED_PACK32;
				case ImageFormat_A8B8G8R8_UINT_PACK32:			return VK_FORMAT_A8B8G8R8_UINT_PACK32;
				case ImageFormat_A8B8G8R8_SINT_PACK32:			return VK_FORMAT_A8B8G8R8_SINT_PACK32;
				case ImageFormat_A8B8G8R8_SRGB_PACK32:			return VK_FORMAT_A8B8G8R8_SRGB_PACK32;
				case ImageFormat_A2R10G10B10_UNORM_PACK32:		return VK_FORMAT_A2R10G10B10_UNORM_PACK32;
				case ImageFormat_A2R10G10B10_SNORM_PACK32:		return VK_FORMAT_A2R10G10B10_SNORM_PACK32;
				case ImageFormat_A2R10G10B10_USCALED_PACK32:	return VK_FORMAT_A2R10G10B10_USCALED_PACK32;
				case ImageFormat_A2R10G10B10_SSCALED_PACK32:	return VK_FORMAT_A2R10G10B10_SSCALED_PACK32;
				case ImageFormat_A2R10G10B10_UINT_PACK32:		return VK_FORMAT_A2R10G10B10_UINT_PACK32;
				case ImageFormat_A2R10G10B10_SINT_PACK32:		return VK_FORMAT_A2R10G10B10_SINT_PACK32;
				case ImageFormat_A2B10G10R10_UNORM_PACK32:		return VK_FORMAT_A2B10G10R10_UNORM_PACK32;
				case ImageFormat_A2B10G10R10_SNORM_PACK32:		return VK_FORMAT_A2B10G10R10_SNORM_PACK32;
				case ImageFormat_A2B10G10R10_USCALED_PACK32:	return VK_FORMAT_A2B10G10R10_USCALED_PACK32;
				case ImageFormat_A2B10G10R10_SSCALED_PACK32:	return VK_FORMAT_A2B10G10R10_SSCALED_PACK32;
				case ImageFormat_A2B10G10R10_UINT_PACK32:		return VK_FORMAT_A2B10G10R10_UINT_PACK32;
				case ImageFormat_A2B10G10R10_SINT_PACK32:		return VK_FORMAT_A2B10G10R10_SINT_PACK32;
				case ImageFormat_R16_UNORM:						return VK_FORMAT_R16_UNORM;
				case ImageFormat_R16_SNORM:						return VK_FORMAT_R16_SNORM;
				case ImageFormat_R16_USCALED:					return VK_FORMAT_R16_USCALED;
				case ImageFormat_R16_SSCALED:					return VK_FORMAT_R16_SSCALED;
				case ImageFormat_R16_UINT:						return VK_FORMAT_R16_UINT;
				case ImageFormat_R16_SINT:						return VK_FORMAT_R16_SINT;
				case ImageFormat_R16_SFLOAT:					return VK_FORMAT_R16_SFLOAT;
				case ImageFormat_R16G16_UNORM:					return VK_FORMAT_R16G16_UNORM;
				case ImageFormat_R16G16_SNORM:					return VK_FORMAT_R16G16_SNORM;
				case ImageFormat_R16G16_USCALED:				return VK_FORMAT_R16G16_USCALED;
				case ImageFormat_R16G16_SSCALED:				return VK_FORMAT_R16G16_SSCALED;
				case ImageFormat_R16G16_UINT:					return VK_FORMAT_R16G16_UINT;
				case ImageFormat_R16G16_SINT:					return VK_FORMAT_R16G16_SINT;
				case ImageFormat_R16G16_SFLOAT:					return VK_FORMAT_R16G16_SFLOAT;
				case ImageFormat_R16G16B16_UNORM:				return VK_FORMAT_R16G16B16_UNORM;
				case ImageFormat_R16G16B16_SNORM:				return VK_FORMAT_R16G16B16_SNORM;
				case ImageFormat_R16G16B16_USCALED:				return VK_FORMAT_R16G16B16_USCALED;
				case ImageFormat_R16G16B16_SSCALED:				return VK_FORMAT_R16G16B16_SSCALED;
				case ImageFormat_R16G16B16_UINT:				return VK_FORMAT_R16G16B16_UINT;
				case ImageFormat_R16G16B16_SINT:				return VK_FORMAT_R16G16B16_SINT;
				case ImageFormat_R16G16B16_SFLOAT:				return VK_FORMAT_R16G16B16_SFLOAT;
				case ImageFormat_R16G16B16A16_UNORM:			return VK_FORMAT_R16G16B16A16_UNORM;
				case ImageFormat_R16G16B16A16_SNORM:			return VK_FORMAT_R16G16B16A16_SNORM;
				case ImageFormat_R16G16B16A16_USCALED:			return VK_FORMAT_R16G16B16A16_USCALED;
				case ImageFormat_R16G16B16A16_SSCALED:			return VK_FORMAT_R16G16B16A16_SSCALED;
				case ImageFormat_R16G16B16A16_UINT:				return VK_FORMAT_R16G16B16A16_UINT;
				case ImageFormat_R16G16B16A16_SINT:				return VK_FORMAT_R16G16B16A16_SINT;
				case ImageFormat_R16G16B16A16_SFLOAT:			return VK_FORMAT_R16G16B16A16_SFLOAT;
				case ImageFormat_R32_UINT:						return VK_FORMAT_R32_UINT;
				case ImageFormat_R32_SINT:						return VK_FORMAT_R32_SINT;
				case ImageFormat_R32_SFLOAT:					return VK_FORMAT_R32_SFLOAT;
				case ImageFormat_R32G32_UINT:					return VK_FORMAT_R32G32_UINT;
				case ImageFormat_R32G32_SINT:					return VK_FORMAT_R32G32_SINT;
				case ImageFormat_R32G32_SFLOAT:					return VK_FORMAT_R32G32_SFLOAT;
				case ImageFormat_R32G32B32_UINT:				return VK_FORMAT_R32G32B32_UINT;
				case ImageFormat_R32G32B32_SINT:				return VK_FORMAT_R32G32B32_SINT;
				case ImageFormat_R32G32B32_SFLOAT:				return VK_FORMAT_R32G32B32_SFLOAT;
				case ImageFormat_R32G32B32A32_UINT:				return VK_FORMAT_R32G32B32A32_UINT;
				case ImageFormat_R32G32B32A32_SINT:				return VK_FORMAT_R32G32B32A32_SINT;
				case ImageFormat_R32G32B32A32_SFLOAT:			return VK_FORMAT_R32G32B32A32_SFLOAT;
				case ImageFormat_R64_UINT:						return VK_FORMAT_R64_UINT;
				case ImageFormat_R64_SINT:						return VK_FORMAT_R64_SINT;
				case ImageFormat_R64_SFLOAT:					return VK_FORMAT_R64_SFLOAT;
				case ImageFormat_R64G64_UINT:					return VK_FORMAT_R64G64_UINT;
				case ImageFormat_R64G64_SINT:					return VK_FORMAT_R64G64_SINT;
				case ImageFormat_R64G64_SFLOAT:					return VK_FORMAT_R64G64_SFLOAT;
				case ImageFormat_R64G64B64_UINT:				return VK_FORMAT_R64G64B64_UINT;
				case ImageFormat_R64G64B64_SINT:				return VK_FORMAT_R64G64B64_SINT;
				case ImageFormat_R64G64B64_SFLOAT:				return VK_FORMAT_R64G64B64_SFLOAT;
				case ImageFormat_R64G64B64A64_UINT:				return VK_FORMAT_R64G64B64A64_UINT;
				case ImageFormat_R64G64B64A64_SINT:				return VK_FORMAT_R64G64B64A64_SINT;
				case ImageFormat_R64G64B64A64_SFLOAT:			return VK_FORMAT_R64G64B64A64_SFLOAT;

				// Depth
				case ImageFormat_D16_UNORM:						return VK_FORMAT_D16_UNORM;
				case ImageFormat_X8_D24_UNORM_PACK32:			return VK_FORMAT_X8_D24_UNORM_PACK32;
				case ImageFormat_D32_SFLOAT:					return VK_FORMAT_D32_SFLOAT;
				case ImageFormat_S8_UINT:						return VK_FORMAT_S8_UINT;
				case ImageFormat_D16_UNORM_S8_UINT:				return VK_FORMAT_D16_UNORM_S8_UINT;
				case ImageFormat_D24_UNORM_S8_UINT:				return VK_FORMAT_D24_UNORM_S8_UINT;
				case ImageFormat_D32_SFLOAT_S8_UINT:			return VK_FORMAT_D32_SFLOAT_S8_UINT;

				default:										return VK_FORMAT_UNDEFINED;
			}
		}

		INLINE i32 GetChannelCountFromVkFormat(VkFormat format) {
			switch (format) {
				// 1 channel (Red only)
				case VK_FORMAT_R8_UNORM: 
				case VK_FORMAT_R8_SRGB:
				case VK_FORMAT_R16_SFLOAT: 
				case VK_FORMAT_R32_SFLOAT:
				case VK_FORMAT_R8_SNORM: 
				case VK_FORMAT_R8_UINT: 
				case VK_FORMAT_R8_SINT:
				case VK_FORMAT_R16_UNORM: 
				case VK_FORMAT_R16_SNORM: 
				case VK_FORMAT_R16_UINT: 
				case VK_FORMAT_R16_SINT:
				case VK_FORMAT_R32_UINT: 
				case VK_FORMAT_R32_SINT:
				return 1;

				// 2 channels (Red, Green)
				case VK_FORMAT_R8G8_UNORM: 
				case VK_FORMAT_R8G8_SRGB:
				case VK_FORMAT_R16G16_SFLOAT: 
				case VK_FORMAT_R32G32_SFLOAT:
				case VK_FORMAT_R8G8_SNORM: 
				case VK_FORMAT_R8G8_UINT:
				case VK_FORMAT_R8G8_SINT:
				case VK_FORMAT_R16G16_UNORM: 
				case VK_FORMAT_R16G16_SNORM: 
				case VK_FORMAT_R16G16_UINT:
				case VK_FORMAT_R16G16_SINT:
				case VK_FORMAT_R32G32_UINT: 
				case VK_FORMAT_R32G32_SINT:
				return 2;

				// 3 channels (Red, Green, Blue)
				case VK_FORMAT_R8G8B8_UNORM: 
				case VK_FORMAT_R8G8B8_SRGB:
				case VK_FORMAT_R16G16B16_SFLOAT: 
				case VK_FORMAT_R32G32B32_SFLOAT:
				case VK_FORMAT_R8G8B8_SNORM: 
				case VK_FORMAT_R8G8B8_UINT: 
				case VK_FORMAT_R8G8B8_SINT:
				case VK_FORMAT_R16G16B16_UNORM: 
				case VK_FORMAT_R16G16B16_SNORM:
				case VK_FORMAT_R16G16B16_UINT: 
				case VK_FORMAT_R16G16B16_SINT:
				case VK_FORMAT_R32G32B32_UINT: 
				case VK_FORMAT_R32G32B32_SINT:
				return 3;

				// 4 channels (Red, Green, Blue, Alpha)
				case VK_FORMAT_R8G8B8A8_UNORM: 
				case VK_FORMAT_R8G8B8A8_SRGB:
				case VK_FORMAT_R16G16B16A16_SFLOAT: 
				case VK_FORMAT_R32G32B32A32_SFLOAT:
				case VK_FORMAT_R8G8B8A8_SNORM: 
				case VK_FORMAT_R8G8B8A8_UINT: 
				case VK_FORMAT_R8G8B8A8_SINT:
				case VK_FORMAT_R16G16B16A16_UNORM: 
				case VK_FORMAT_R16G16B16A16_SNORM:
				case VK_FORMAT_R16G16B16A16_UINT: 
				case VK_FORMAT_R16G16B16A16_SINT:
				case VK_FORMAT_R32G32B32A32_UINT: 
				case VK_FORMAT_R32G32B32A32_SINT:
				case VK_FORMAT_B8G8R8A8_UNORM:
				case VK_FORMAT_B8G8R8A8_SRGB:
				return 4;

				default:
					return VK_FORMAT_UNDEFINED;
			}
		}

	} // namespace graphics

} // namespace hellengine