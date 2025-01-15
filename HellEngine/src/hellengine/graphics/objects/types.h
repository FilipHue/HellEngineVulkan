#pragma once

// STL
#include <optional>

namespace hellengine
{

	namespace graphics
	{

		/**********\
		|  DEVICE  |
		\**********/

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

		/************\
		| SWAPCHAIN  |
		\************/

		struct SwapChainSupportDetails
		{
			VkSurfaceCapabilitiesKHR capabilities;
			std::vector<VkSurfaceFormatKHR> formats;
			std::vector<VkPresentModeKHR> present_modes;
		};

		/************\
		| RENDERING  |
		\************/

		struct RenderData
		{
			VkExtent2D extent;
			VkClearValue clear_value;
			VkClearValue depth_clear_value;
		};

		/**************\
		| SHADER STAGE |
		\**************/

		enum ShaderType
		{
			ShaderType_Undefined,
			ShaderType_Vertex,
			ShaderType_Fragment,
			ShaderType_Compute,
			ShaderType_Geometry,

			ShaderType_Count
		};

		enum ShaderStage
		{
			ShaderStage_Vertex =	1,
			ShaderStage_Fragment =	2,
			ShaderStage_Compute =	4,
			ShaderStage_Geometry =	8,

			ShaderStage_All =		ShaderStage_Vertex | ShaderStage_Fragment | ShaderStage_Compute | ShaderStage_Geometry,

			ShaderStage_Count
		};

		struct ShaderStageInfo
		{
			std::unordered_map<ShaderType, std::string> sources;
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

		struct DescriptorSetWriteData
		{
			union WriteData
			{
				struct Buffer
				{
					VkBuffer buffer;
					VkDeviceSize offset;
					VkDeviceSize range;
				} buffer;

				struct Image
				{
					VkImageView image_view;
					VkSampler sampler;
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

		struct DescriptorLayoutBindingInfo
		{
			u32 binding;
			DescriptorType type;
			ShaderStage stage;
		};

		/**********\
		| PIPELINE |
		\**********/

		enum PipelineType
		{
			PipelineType_None,
			PipelineType_Graphics,
			PipelineType_Compute,
			PipelineType_RayTracing,

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
			PipelinePolygonMode_Fill,
			PipelinePolygonMode_Line,
			PipelinePolygonMode_Point,

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

		struct PipelineCreateInfo
		{
			PipelinePrimitiveTopology topology;
			PipelinePolygonMode polygon_mode;
			PipelineCullMode cull_mode;
			PipelineFrontFace front_face;
			f32 line_width;

			std::vector<PipelineDynamicState> dynamic_states;
			std::vector<std::vector<DescriptorLayoutBindingInfo>> layouts;
			VkVertexInputBindingDescription vertex_binding_description;
			std::vector<VkVertexInputAttributeDescription> vertex_attribute_descriptions;
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

			BufferType_Count
		};

	} // namespace graphics

} // namespace hellengine