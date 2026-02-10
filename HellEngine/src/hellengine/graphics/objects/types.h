#pragma once

// STL
#include <optional>

namespace hellengine
{

	namespace graphics
	{

		/********\
		| DEVICE |
		\********/

		struct DeviceRequirements
		{
			b8 graphics = false;
			b8 compute = false;
			b8 video_decode = false;
			b8 video_encode = false;
			b8 protected_memory_management = false;
			b8 sparse_memory_management = false;
			b8 transfer = false;

			b8 discrete = false;
			b8 present_support = false;

			std::vector<const char*> extensions = {};
		};

		struct QueueFamilyIndices
		{
			std::optional<u32> graphics_family;
			std::optional<u32> compute_family;
			std::optional<u32> transfer_family;
			std::optional<u32> present_family;

			b8 IsComplete() const
			{
				return graphics_family.has_value() &&
					compute_family.has_value() &&
					transfer_family.has_value() &&
					present_family.has_value();
			}
		};

		struct DeviceQueues
		{
			VkQueue graphics_queue;
			VkQueue compute_queue;
			VkQueue transfer_queue;
			VkQueue present_queue;
		};

		/***********\
		| SWAPCHAIN |
		\***********/

		struct SwapChainSupportDetails
		{
			VkSurfaceCapabilitiesKHR capabilities;
			std::vector<VkSurfaceFormatKHR> formats;
			std::vector<VkPresentModeKHR> present_modes;
		};

		/***********\
		| RENDERING |
		\***********/

		struct RenderData
		{
			VkExtent2D extent;
			VkClearValue clear_value;
			VkClearValue depth_clear_value;
		};

		/*******************\
		| DYNAMIC RENDERING |
		\*******************/

		struct DynamicRenderingAttachmentInfo
		{
			VkImage image;
			VkImageView image_view;
			VkFormat format;
			VkImageLayout image_layout;
			VkAttachmentLoadOp load_op;
			VkAttachmentStoreOp store_op;
			VkClearValue clear_value;
			VkImageLayout initial_layout;
			VkImageLayout final_layout;
		};

		struct DynamicRenderingInfo
		{
			std::vector<DynamicRenderingAttachmentInfo> color_attachments;
			std::optional<DynamicRenderingAttachmentInfo> depth_attachment;
			VkRenderingFlags flags{};
			VkExtent2D extent{};
		};

		/*************\
		| FRAMEBUFFER |
		\*************/

		struct FramebufferInfo
		{
			u32 attachment_count;
			VkImageView* attachments;
			u32 width;
			u32 height;
			u32 layers;
		};

		/************\
		| RENDERPASS |
		\************/

		enum AttachmentLoadOp
		{
			AttachmentLoadOp_Load,
			AttachmentLoadOp_Clear,
			AttachmentLoadOp_DontCare,

			AttachmentLoadOp_Count
		};

		enum AttachmentStoreOp
		{
			AttachmentStoreOp_Store,
			AttachmentStoreOp_DontCare,

			AttachmentStoreOp_Count
		};

		enum RenderPassType
		{
			RenderPassType_Offscreen,
			RenderPassType_Swapchain,

			RenderPassType_Count
		};

		struct RenderPassAttachmentInfo
		{
			VkFormat format;
			AttachmentLoadOp load_op;
			AttachmentStoreOp store_op;
			AttachmentLoadOp stencil_load_op;
			AttachmentStoreOp stencil_store_op;
			VkImageLayout initial_layout;
			VkImageLayout final_layout;

			b8 is_swapchain_attachment = false;
			b8 is_depth_attachment = false;
			b8 is_stencil_attachment = false;
			b8 is_depth_stencil_attachment = false;
			VkImageView image_view;
		};

		struct RenderSubpassInfo
		{
			VkPipelineBindPoint pipeline_bind_point;

			std::vector<VkAttachmentReference> color_attachments;
			std::optional<VkAttachmentReference> depth_attachment;
		};

		struct RenderPassDepencyInfo
		{
			u32 src_subpass;
			u32 dst_subpass;
			VkPipelineStageFlags src_stage_mask;
			VkPipelineStageFlags dst_stage_mask;
			VkAccessFlags src_access_mask;
			VkAccessFlags dst_access_mask;
			VkDependencyFlags dependency_flags;
		};

		struct RenderPassInfo
		{
			RenderPassType type;

			std::vector<RenderPassAttachmentInfo> attachments;
			std::vector<RenderSubpassInfo> subpasses;
			std::vector<RenderPassDepencyInfo> dependencies;

			std::vector<VkImageView> custom_attachment_views;

			u32 width;
			u32 height;
		};

		/**************\
		| SHADER STAGE |
		\**************/

		enum ShaderType
		{
			ShaderType_Vertex,
			ShaderType_TessControl,
			ShaderType_TessEvaluation,
			ShaderType_Geometry,
			ShaderType_Fragment,
			ShaderType_Compute,

			ShaderType_Count
		};

		enum ShaderStage
		{
			ShaderStage_Vertex			= 1,
			ShaderStage_TessControl     = 2,
			ShaderStage_TessEvaluation  = 4,
			ShaderStage_Geometry		= 8,
			ShaderStage_Fragment		= 16,
			ShaderStage_Compute			= 32,

			ShaderStage_All = ShaderStage_Vertex | ShaderStage_TessControl | ShaderStage_TessEvaluation | ShaderStage_Geometry | ShaderStage_Fragment | ShaderStage_Compute,

			ShaderStage_Count
		};

		struct ShaderSpecializationEntryInfo
		{
			u32 id;
			u32 size;
			u32 offset;
		};

		struct ShaderSpecializationInfo
		{
			ShaderStage stage;
			u32 size;
			const void* data;
			std::vector<ShaderSpecializationEntryInfo> entries;
		};

		struct ShaderStageInfo
		{
			std::unordered_map<ShaderType, std::string> sources;
			std::vector<ShaderSpecializationInfo> specialization_infos;
		};

		/*************\
		| DESCRIPTORS |
		\*************/

		enum DescriptorType
		{
			DescriptorType_UniformBuffer,
			DescriptorType_StorageBuffer,
			DescriptorType_UniformTexelBuffer,
			DescriptorType_StorageTexelBuffer,
			DescriptorType_UniformBufferDynamic,
			DescriptorType_StorageBufferDynamic,
			DescriptorType_InputAttachment,
			DescriptorType_StorageImage,
			DescriptorType_SampledImage,
			DescriptorType_CombinedImageSampler,
			DescriptorType_Sampler,

			DescriptorType_Count
		};

		enum DescriptorBindingFlags
		{
			DescriptorBindingFlags_None							= 0,
			DescriptorBindingFlags_UpdateAfterBind				= 1,
			DescriptorBindingFlags_UpdateUnusedWhilePending		= 2,
			DescriptorBindingFlags_PartiallyBound				= 4,
			DescriptorBindingFlags_VariableCount				= 8,

			DescriptorBindingFlags_Count
		};

		enum DescriptorSetFlags
		{
			DescriptorSetFlags_None					= 0,
			DescriptorSetFlags_UpdateAfterBindPool	= 2
		};

		struct DescriptorSetWriteData
		{
			union WriteData
			{
				struct Buffer
				{
					VkBuffer* buffers;
					VkDeviceSize* offsets;
					VkDeviceSize* ranges;
				} buffer;

				struct Image
				{
					VkImageView* image_views;
					VkSampler* samplers;
				} image;
			} data;

			u32 binding;
			DescriptorType type;
		};

		struct DescriptorPoolSizeInfo
		{
			DescriptorType type;
			u32 count;
		};

		struct DescriptorSetBindingInfo
		{
			u32 binding;
			DescriptorType type;
			u32 count;
			ShaderStage stage;
			DescriptorBindingFlags flags;
		};

		struct DescriptorSetInfo
		{
			std::vector<DescriptorSetBindingInfo> bindings;
			DescriptorSetFlags flags;
		};

		/**********\
		| PIPELINE |
		\**********/

		enum PipelineType
		{
			PipelineType_None,
			PipelineType_Graphics,
			PipelineType_Compute,

			PipelineType_Count
		};

		enum PipelinePrimitiveTopology
		{
			PipelinePrimitiveTopology_PointList,
			PipelinePrimitiveTopology_LineList,
			PipelinePrimitiveTopology_LineStrip,
			PipelinePrimitiveTopology_TriangleList,
			PipelinePrimitiveTopology_TriangleStrip,
			PipelinePrimitiveTopology_TriangleFan,

			PipelinePrimitiveTopology_Count
		};

		enum PipelinePolygonMode
		{
			PipelinePolygonMode_Point,
			PipelinePolygonMode_Line,
			PipelinePolygonMode_Fill,

			PipelinePolygonMode_Count
		};

		enum PipelineCullMode
		{
			PipelineCullMode_None,
			PipelineCullMode_Front,
			PipelineCullMode_Back,
			PipelineCullMode_FrontAndBack,

			PipelineCullMode_Count
		};

		enum PipelineFrontFace
		{
			PipelineFrontFace_Clockwise,
			PipelineFrontFace_CounterClockwise,

			PipelineFrontFace_Count
		};

		enum PipelineDynamicState
		{
			PipelineDynamicState_Viewport,
			PipelineDynamicState_Scissor,
			PipelineDynamicState_LineWidth,
			PipelineDynamicState_DepthBias,
			PipelineDynamicState_BlendConstants,
			PipelineDynamicState_DepthBounds,
			PipelineDynamicState_StencilCompareMask,
			PipelineDynamicState_StencilWriteMask,
			PipelineDynamicState_StencilReference,

			PipelineDynamicState_Count
		};

		enum PipelineDethStencilCompareOp
		{
			PipelineDethStencilCompareOp_Never,
			PipelineDethStencilCompareOp_Less,
			PipelineDethStencilCompareOp_Equal,
			PipelineDethStencilCompareOp_LessOrEqual,
			PipelineDethStencilCompareOp_Greater,
			PipelineDethStencilCompareOp_NotEqual,
			PipelineDethStencilCompareOp_GreaterOrEqual,
			PipelineDethStencilCompareOp_Always,

			PipelineDethStencilCompareOp_Count
		};

		struct PipelinePushConstantRange
		{
			ShaderStage stage;
			u32 offset;
			u32 size;
		};

		struct PipelineDepthStencilInfo
		{
			b8 depth_test_enable = true;
			b8 depth_write_enable = true;
			b8 stencil_test_enable = false;
			PipelineDethStencilCompareOp depth_compare_op = PipelineDethStencilCompareOp_Less;

			VkStencilOpState front{};
			VkStencilOpState back{};
		};

		struct PipelineDynamicRenderingInfo
		{
			b8 blend_enable = false;
			std::vector<VkFormat> color_formats;
			std::optional<VkFormat> depth_format;
			std::optional<VkFormat> stencil_format;
		};

		struct PipelineRenderPassInfo
		{
			VkRenderPass render_pass;
			u32 subpass;
		};

		struct PipelineCreateInfo
		{
			PipelineType type;
			PipelinePrimitiveTopology topology;
			PipelinePolygonMode polygon_mode;
			PipelineCullMode cull_mode;
			PipelineFrontFace front_face;
			f32 line_width;

			std::vector<PipelineDynamicState> dynamic_states;
			std::vector<DescriptorSetInfo> layout;
			VkVertexInputBindingDescription vertex_binding_description;
			std::vector<VkVertexInputAttributeDescription> vertex_attribute_descriptions;
			std::vector<PipelinePushConstantRange> push_constant_ranges;
			PipelineDepthStencilInfo depth_stencil_info;
			std::optional<PipelineDynamicRenderingInfo> dynamic_rendering_info;
			std::optional<PipelineRenderPassInfo> renderpass_rendering_info;
		};

		/********\
		| BUFFER |
		\********/

		enum BufferType
		{
			BufferType_None,
			BufferType_Stagging,
			BufferType_Vertex,
			BufferType_Index,
			BufferType_Uniform,
			BufferType_UniformDynamic,
			BufferType_Storage,
			BufferType_DrawIndirect,

			BufferType_Count
		};

		enum BufferMemoryProperty
		{
			BufferMemoryProperty_None			= 0,
			BufferMemoryProperty_HostCoherent	= 1 << 0,	// VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
			BufferMemoryProperty_HostCached		= 1 << 1,	// VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT
			BufferMemoryProperty_DeviceLocal	= 1 << 2,	// VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		};

		/*******\
		| IMAGE |
		\*******/

		enum ImageFilterMode
		{
			ImageFilterMode_Nearest,
			ImageFilterMode_Linear,

			ImageFilterMode_Count
		};

		enum ImageAddressMode
		{
			ImageAddressMode_Repeat,
			ImageAddressMode_MirroredRepeat,
			ImageAddressMode_ClampToEdge,
			ImageAddressMode_ClampToBorder,
			ImageAddressMode_MirroredClampToEdge,

			ImageAddressMode_Count
		};

		enum ImageCompareOp
		{
			ImageCompareOp_Never,
			ImageCompareOp_Less,
			ImageCompareOp_Equal,
			ImageCompareOp_LessOrEqual,
			ImageCompareOp_Greater,
			ImageCompareOp_NotEqual,
			ImageCompareOp_GreaterOrEqual,
			ImageCompareOp_Always,

			ImageCompareOp_Count
		};

		enum ImageBorderColor
		{
			ImageBorderColor_FloatTransparentBlack,
			ImageBorderColor_IntTransparentBlack,
			ImageBorderColor_FloatOpaqueBlack,
			ImageBorderColor_IntOpaqueBlack,
			ImageBorderColor_FloatOpaqueWhite,
			ImageBorderColor_IntOpaqueWhite,

			ImageBorderColor_Count
		};

		enum ImageFormat
		{
			ImageFormat_UNDEFINED = 0,
			ImageFormat_R4G4_UNORM_PACK8,
			ImageFormat_R4G4B4A4_UNORM_PACK16,
			ImageFormat_B4G4R4A4_UNORM_PACK16,
			ImageFormat_R5G6B5_UNORM_PACK16,
			ImageFormat_B5G6R5_UNORM_PACK16,
			ImageFormat_R5G5B5A1_UNORM_PACK16,
			ImageFormat_B5G5R5A1_UNORM_PACK16,
			ImageFormat_A1R5G5B5_UNORM_PACK16,
			ImageFormat_R8_UNORM,
			ImageFormat_R8_SNORM,
			ImageFormat_R8_USCALED,
			ImageFormat_R8_SSCALED,
			ImageFormat_R8_UINT,
			ImageFormat_R8_SINT,
			ImageFormat_R8_SRGB,
			ImageFormat_R8G8_UNORM,
			ImageFormat_R8G8_SNORM,
			ImageFormat_R8G8_USCALED,
			ImageFormat_R8G8_SSCALED,
			ImageFormat_R8G8_UINT,
			ImageFormat_R8G8_SINT,
			ImageFormat_R8G8_SRGB,
			ImageFormat_R8G8B8_UNORM,
			ImageFormat_R8G8B8_SNORM,
			ImageFormat_R8G8B8_USCALED,
			ImageFormat_R8G8B8_SSCALED,
			ImageFormat_R8G8B8_UINT,
			ImageFormat_R8G8B8_SINT,
			ImageFormat_R8G8B8_SRGB,
			ImageFormat_B8G8R8_UNORM,
			ImageFormat_B8G8R8_SNORM,
			ImageFormat_B8G8R8_USCALED,
			ImageFormat_B8G8R8_SSCALED,
			ImageFormat_B8G8R8_UINT,
			ImageFormat_B8G8R8_SINT,
			ImageFormat_B8G8R8_SRGB,
			ImageFormat_R8G8B8A8_UNORM,
			ImageFormat_R8G8B8A8_SNORM,
			ImageFormat_R8G8B8A8_USCALED,
			ImageFormat_R8G8B8A8_SSCALED,
			ImageFormat_R8G8B8A8_UINT,
			ImageFormat_R8G8B8A8_SINT,
			ImageFormat_R8G8B8A8_SRGB,
			ImageFormat_B8G8R8A8_UNORM,
			ImageFormat_B8G8R8A8_SNORM,
			ImageFormat_B8G8R8A8_USCALED,
			ImageFormat_B8G8R8A8_SSCALED,
			ImageFormat_B8G8R8A8_UINT,
			ImageFormat_B8G8R8A8_SINT,
			ImageFormat_B8G8R8A8_SRGB,
			ImageFormat_A8B8G8R8_UNORM_PACK32,
			ImageFormat_A8B8G8R8_SNORM_PACK32,
			ImageFormat_A8B8G8R8_USCALED_PACK32,
			ImageFormat_A8B8G8R8_SSCALED_PACK32,
			ImageFormat_A8B8G8R8_UINT_PACK32,
			ImageFormat_A8B8G8R8_SINT_PACK32,
			ImageFormat_A8B8G8R8_SRGB_PACK32,
			ImageFormat_A2R10G10B10_UNORM_PACK32,
			ImageFormat_A2R10G10B10_SNORM_PACK32,
			ImageFormat_A2R10G10B10_USCALED_PACK32,
			ImageFormat_A2R10G10B10_SSCALED_PACK32,
			ImageFormat_A2R10G10B10_UINT_PACK32,
			ImageFormat_A2R10G10B10_SINT_PACK32,
			ImageFormat_A2B10G10R10_UNORM_PACK32,
			ImageFormat_A2B10G10R10_SNORM_PACK32,
			ImageFormat_A2B10G10R10_USCALED_PACK32,
			ImageFormat_A2B10G10R10_SSCALED_PACK32,
			ImageFormat_A2B10G10R10_UINT_PACK32,
			ImageFormat_A2B10G10R10_SINT_PACK32,
			ImageFormat_R16_UNORM,
			ImageFormat_R16_SNORM,
			ImageFormat_R16_USCALED,
			ImageFormat_R16_SSCALED,
			ImageFormat_R16_UINT,
			ImageFormat_R16_SINT,
			ImageFormat_R16_SFLOAT,
			ImageFormat_R16G16_UNORM,
			ImageFormat_R16G16_SNORM,
			ImageFormat_R16G16_USCALED,
			ImageFormat_R16G16_SSCALED,
			ImageFormat_R16G16_UINT,
			ImageFormat_R16G16_SINT,
			ImageFormat_R16G16_SFLOAT,
			ImageFormat_R16G16B16_UNORM,
			ImageFormat_R16G16B16_SNORM,
			ImageFormat_R16G16B16_USCALED,
			ImageFormat_R16G16B16_SSCALED,
			ImageFormat_R16G16B16_UINT,
			ImageFormat_R16G16B16_SINT,
			ImageFormat_R16G16B16_SFLOAT,
			ImageFormat_R16G16B16A16_UNORM,
			ImageFormat_R16G16B16A16_SNORM,
			ImageFormat_R16G16B16A16_USCALED,
			ImageFormat_R16G16B16A16_SSCALED,
			ImageFormat_R16G16B16A16_UINT,
			ImageFormat_R16G16B16A16_SINT,
			ImageFormat_R16G16B16A16_SFLOAT,
			ImageFormat_R32_UINT,
			ImageFormat_R32_SINT,
			ImageFormat_R32_SFLOAT,
			ImageFormat_R32G32_UINT,
			ImageFormat_R32G32_SINT,
			ImageFormat_R32G32_SFLOAT,
			ImageFormat_R32G32B32_UINT,
			ImageFormat_R32G32B32_SINT,
			ImageFormat_R32G32B32_SFLOAT,
			ImageFormat_R32G32B32A32_UINT,
			ImageFormat_R32G32B32A32_SINT,
			ImageFormat_R32G32B32A32_SFLOAT,
			ImageFormat_R64_UINT,
			ImageFormat_R64_SINT,
			ImageFormat_R64_SFLOAT,
			ImageFormat_R64G64_UINT,
			ImageFormat_R64G64_SINT,
			ImageFormat_R64G64_SFLOAT,
			ImageFormat_R64G64B64_UINT,
			ImageFormat_R64G64B64_SINT,
			ImageFormat_R64G64B64_SFLOAT,
			ImageFormat_R64G64B64A64_UINT,
			ImageFormat_R64G64B64A64_SINT,
			ImageFormat_R64G64B64A64_SFLOAT,

			// Depth formats
			ImageFormat_D16_UNORM,
			ImageFormat_X8_D24_UNORM_PACK32,
			ImageFormat_D32_SFLOAT,
			ImageFormat_S8_UINT,
			ImageFormat_D16_UNORM_S8_UINT,
			ImageFormat_D24_UNORM_S8_UINT,
			ImageFormat_D32_SFLOAT_S8_UINT,

			ImageFormat_Count
		};


		struct ImageSamplerCreationInfo
		{
			ImageFilterMode mag_filter = ImageFilterMode_Linear;
			ImageFilterMode min_filter = ImageFilterMode_Linear;
			ImageAddressMode address_mode[3] = { ImageAddressMode_Repeat, ImageAddressMode_Repeat, ImageAddressMode_Repeat };
			std::optional<ImageCompareOp> compare_op = std::nullopt;
			f32 min_lod = 0.0f;
			f32 max_lod = 1.0f;
			ImageBorderColor border_color = ImageBorderColor_FloatTransparentBlack;
		};

	} // namespace graphics

} // namespace hellengine