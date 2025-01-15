#include "hepch.h"
#include "vulkan_pipeline.h"

// Internal
#include "vulkan_instance.h"
#include "vulkan_device.h"
#include "vulkan_shader.h"
#include "vulkan_swapchain.h"

#include <hellengine/math/types.h>
#include <hellengine/resources/file_manager.h>

namespace hellengine
{

	using namespace math;
	namespace graphics
	{

		VulkanPipeline::VulkanPipeline()
		{
			m_handle = VK_NULL_HANDLE;
			m_layout = VK_NULL_HANDLE;

			m_shader_stages = {};
			m_input_assembly_state = {};
			m_viewport_state = {};
			m_rasterization_state = {};
			m_multisample_state = {};
			m_depth_stencil_state = {};
			m_color_blend_attachment = {};
			m_color_blend_state = {};
			m_dynamic_state = {};
			m_vertex_input_state = {};

			m_type = PipelineType_None;
		}

		VulkanPipeline::~VulkanPipeline()
		{
			NO_OP;
		}

		void VulkanPipeline::Create(const VulkanInstance& instance, VulkanDevice& device, VulkanSwapchain& swapchain, const PipelineCreateInfo& info, const ShaderStageInfo& shader_info)
		{
			SetPipelineLayout(instance, device, info);
			SetPipelineState(instance, device, info);
			CreateShaderStages(instance, device, shader_info);

			VkPipelineRenderingCreateInfo rendering_info = {};
			rendering_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
			rendering_info.pNext = VK_NULL_HANDLE;
			rendering_info.colorAttachmentCount = 1;

			VkFormat format = swapchain.GetImageFormat();
			rendering_info.pColorAttachmentFormats = &format;
			rendering_info.depthAttachmentFormat = device.GetDepthFormat();
			rendering_info.stencilAttachmentFormat = VK_FORMAT_UNDEFINED;

			VkGraphicsPipelineCreateInfo pipeline_info = {};
			pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
			pipeline_info.pNext = &rendering_info;
			pipeline_info.flags = 0;
			pipeline_info.stageCount = (u32)m_shader_stages.size();
			pipeline_info.pStages = m_shader_stages.data();
			pipeline_info.pVertexInputState = &m_vertex_input_state;
			pipeline_info.pInputAssemblyState = &m_input_assembly_state;
			pipeline_info.pTessellationState = VK_NULL_HANDLE;
			pipeline_info.pViewportState = &m_viewport_state;
			pipeline_info.pRasterizationState = &m_rasterization_state;
			pipeline_info.pMultisampleState = &m_multisample_state;
			pipeline_info.pDepthStencilState = &m_depth_stencil_state;
			pipeline_info.pColorBlendState = &m_color_blend_state;
			pipeline_info.pDynamicState = &m_dynamic_state;
			pipeline_info.layout = m_layout;
			pipeline_info.renderPass = VK_NULL_HANDLE;
			pipeline_info.subpass = 0;
			pipeline_info.basePipelineHandle = VK_NULL_HANDLE;
			pipeline_info.basePipelineIndex = -1;

			VK_CHECK(vkCreateGraphicsPipelines(device.GetLogicalDevice(), VK_NULL_HANDLE, 1, &pipeline_info, instance.GetAllocator(), &m_handle));
		
			DestroyShaderStages(instance, device);
		}

		void VulkanPipeline::Destroy(const VulkanInstance& instance, const VulkanDevice& device) const
		{
			for (auto& layout : m_descriptor_set_layouts)
			{
				vkDestroyDescriptorSetLayout(device.GetLogicalDevice(), layout, instance.GetAllocator());
			}
			vkDestroyPipelineLayout(device.GetLogicalDevice(), m_layout, instance.GetAllocator());
			vkDestroyPipeline(device.GetLogicalDevice(), m_handle, instance.GetAllocator());
		}

		void VulkanPipeline::Bind(const VulkanDevice& device, VkCommandBuffer command_buffer) const
		{
			vkCmdBindPipeline(command_buffer, GetVulkanBindPoint(m_type), m_handle);
		}

		void VulkanPipeline::SetPipelineLayout(const VulkanInstance& instance, const VulkanDevice& device, const PipelineCreateInfo& info)
		{
			for (const auto& layout : info.layouts)
			{
				std::vector<VkDescriptorSetLayoutBinding> bindings_layout = {};
				for (const auto& binding : layout)
				{
					VkDescriptorSetLayoutBinding layout_binding = {};
					layout_binding.binding = binding.binding;
					layout_binding.descriptorType = ToVulkanDescriptorType(binding.type);
					layout_binding.descriptorCount = 1;
					layout_binding.stageFlags = ToVulkanShaderStageFlags(binding.stage);
					layout_binding.pImmutableSamplers = VK_NULL_HANDLE;

					bindings_layout.push_back(layout_binding);
				}

				VkDescriptorSetLayout layout = VK_NULL_HANDLE;

				VkDescriptorSetLayoutCreateInfo layout_info = {};
				layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
				layout_info.pNext = VK_NULL_HANDLE;
				layout_info.flags = 0;
				layout_info.bindingCount = (u32)bindings_layout.size();
				layout_info.pBindings = bindings_layout.data();

				VK_CHECK(vkCreateDescriptorSetLayout(device.GetLogicalDevice(), &layout_info, instance.GetAllocator(), &layout));
				m_descriptor_set_layouts.push_back(layout);
			}

			VkPipelineLayoutCreateInfo pipeline_layout_info = {};
			pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
			pipeline_layout_info.pNext = VK_NULL_HANDLE;
			pipeline_layout_info.flags = 0;
			pipeline_layout_info.setLayoutCount = (u32)m_descriptor_set_layouts.size();
			pipeline_layout_info.pSetLayouts = m_descriptor_set_layouts.data();
			pipeline_layout_info.pushConstantRangeCount = 0;
			pipeline_layout_info.pPushConstantRanges = VK_NULL_HANDLE;

			VK_CHECK(vkCreatePipelineLayout(device.GetLogicalDevice(), &pipeline_layout_info, instance.GetAllocator(), &m_layout));
		}

		void VulkanPipeline::SetPipelineState(const VulkanInstance& instance, const VulkanDevice& device, const PipelineCreateInfo& info)
		{
			for (u32 i = 0; i < info.dynamic_states.size(); i++)
			{
				m_dynamic_states.push_back(GetVulkanDynamicState(info.dynamic_states[i]));
			}

			m_dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
			m_dynamic_state.pNext = VK_NULL_HANDLE;
			m_dynamic_state.flags = 0;
			m_dynamic_state.dynamicStateCount = (u32)m_dynamic_states.size();
			m_dynamic_state.pDynamicStates = m_dynamic_states.data();

			m_input_assembly_state.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
			m_input_assembly_state.pNext = VK_NULL_HANDLE;
			m_input_assembly_state.flags = 0;
			m_input_assembly_state.topology = GetVulkanTopology(info.topology);
			m_input_assembly_state.primitiveRestartEnable = VK_FALSE;

			m_viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
			m_viewport_state.pNext = VK_NULL_HANDLE;
			m_viewport_state.flags = 0;
			m_viewport_state.viewportCount = 1;
			m_viewport_state.pViewports = VK_NULL_HANDLE;
			m_viewport_state.scissorCount = 1;
			m_viewport_state.pScissors = VK_NULL_HANDLE;

			m_rasterization_state.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
			m_rasterization_state.pNext = VK_NULL_HANDLE;
			m_rasterization_state.flags = 0;
			m_rasterization_state.depthClampEnable = VK_FALSE;
			m_rasterization_state.rasterizerDiscardEnable = VK_FALSE;
			m_rasterization_state.polygonMode = GetVulkanPolygonMode(info.polygon_mode);
			m_rasterization_state.lineWidth = info.line_width;
			m_rasterization_state.cullMode = GetVulkanCullMode(info.cull_mode);
			m_rasterization_state.frontFace = GetVulkanFrontFace(info.front_face);
			m_rasterization_state.depthBiasEnable = VK_FALSE;
			m_rasterization_state.depthBiasConstantFactor = 0.0f;
			m_rasterization_state.depthBiasClamp = 0.0f;
			m_rasterization_state.depthBiasSlopeFactor = 0.0f;

			m_multisample_state.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
			m_multisample_state.pNext = VK_NULL_HANDLE;
			m_multisample_state.flags = 0;
			m_multisample_state.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
			m_multisample_state.sampleShadingEnable = VK_FALSE;
			m_multisample_state.minSampleShading = 1.0f;
			m_multisample_state.pSampleMask = VK_NULL_HANDLE;
			m_multisample_state.alphaToCoverageEnable = VK_FALSE;
			m_multisample_state.alphaToOneEnable = VK_FALSE;

			m_depth_stencil_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
			m_depth_stencil_state.pNext = VK_NULL_HANDLE;
			m_depth_stencil_state.flags = 0;
			m_depth_stencil_state.depthTestEnable = VK_TRUE;
			m_depth_stencil_state.depthWriteEnable = VK_TRUE;
			m_depth_stencil_state.depthCompareOp = VK_COMPARE_OP_LESS;
			m_depth_stencil_state.depthBoundsTestEnable = VK_FALSE;
			m_depth_stencil_state.stencilTestEnable = VK_FALSE;
			m_depth_stencil_state.front = {};
			m_depth_stencil_state.back = {};
			m_depth_stencil_state.minDepthBounds = 0.0f;
			m_depth_stencil_state.maxDepthBounds = 1.0f;

			m_color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
			m_color_blend_attachment.blendEnable = VK_FALSE;
			m_color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
			m_color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
			m_color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
			m_color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
			m_color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
			m_color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;

			m_color_blend_state.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
			m_color_blend_state.pNext = VK_NULL_HANDLE;
			m_color_blend_state.flags = 0;
			m_color_blend_state.logicOpEnable = VK_FALSE;
			m_color_blend_state.logicOp = VK_LOGIC_OP_COPY;
			m_color_blend_state.attachmentCount = 1;
			m_color_blend_state.pAttachments = &m_color_blend_attachment;
			m_color_blend_state.blendConstants[0] = 0.0f;
			m_color_blend_state.blendConstants[1] = 0.0f;
			m_color_blend_state.blendConstants[2] = 0.0f;
			m_color_blend_state.blendConstants[3] = 0.0f;

			m_vertex_input_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
			m_vertex_input_state.pNext = VK_NULL_HANDLE;
			m_vertex_input_state.flags = 0;
			m_vertex_input_state.vertexBindingDescriptionCount = 1;
			m_vertex_input_state.pVertexBindingDescriptions = &info.vertex_binding_description;
			m_vertex_input_state.vertexAttributeDescriptionCount = static_cast<u32>(info.vertex_attribute_descriptions.size());
			m_vertex_input_state.pVertexAttributeDescriptions = info.vertex_attribute_descriptions.data();
		}

		void VulkanPipeline::CreateShaderStages(const VulkanInstance& instance, const VulkanDevice& device, const ShaderStageInfo& shader_info)
		{
			for (auto& [type, path] : shader_info.sources)
			{
				VulkanShader shader = VulkanShader();
				resources::File file = resources::FileManager::ReadFile(path.c_str());
				shader.Create(instance, device, file, type);
				m_shader_stages.push_back(VulkanShader::GetShaderStageCreateInfo(shader));
			}
		}

		void VulkanPipeline::DestroyShaderStages(const VulkanInstance& instance, const VulkanDevice& device)
		{
			for (u32 i = 0; i < m_shader_stages.size(); i++)
			{
				VulkanShader::DestroyShaderStageCreateInfo(instance, device, m_shader_stages[i]);
			}
		}

	} // namespace graphics

} // namespace hellengine