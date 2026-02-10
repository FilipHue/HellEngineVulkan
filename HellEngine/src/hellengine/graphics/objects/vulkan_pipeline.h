#pragma once

// Internal
#include "shared.h"

namespace hellengine
{

	namespace graphics
	{

		class VulkanPipeline
		{
		public:
			VulkanPipeline();
			~VulkanPipeline();

			void CreateGraphics(const VulkanInstance& instance, VulkanDevice& device, VulkanSwapchain& swapchain, const PipelineCreateInfo& info, const ShaderStageInfo& shader_info);
			void CreateCompute(const VulkanInstance& instance, VulkanDevice& device, const PipelineCreateInfo& info, const ShaderStageInfo& shader_info);

			void Destroy(const VulkanInstance& instance, const VulkanDevice& device) const;

			void Bind(const VulkanDevice& device, VkCommandBuffer command_buffer) const;

			VkPipeline GetHandle() const { return m_handle; }
			VkPipelineLayout GetLayout() const { return m_layout; }
			VkDescriptorSetLayout GetDescriptorSetLayout(u32 descriptor_index) const;
			VkPipelineBindPoint GetBindPoint() const { return GetVulkanBindPoint(m_type); }
			PipelineType GetType() const { return m_type; }

			VkPipelineRenderingCreateInfoKHR GetRenderingCreateInfo() const
			{
				if (m_rendering_create_info.has_value())
					return m_rendering_create_info.value();
				else
					return {};
			}

			void SetPipelineLayout(const VulkanInstance& instance, const VulkanDevice& device, const PipelineCreateInfo& info);
			void SetPipelineState(const VulkanInstance& instance, const VulkanDevice& device, const PipelineCreateInfo& info);
			void SetType(PipelineType type) { m_type = type; }

			void CreateShaderStages(const VulkanInstance& instance, const VulkanDevice& device, const ShaderStageInfo& shader_info);
			void DestroyShaderStages(const VulkanInstance& instance, const VulkanDevice& device);

		private:
			VkPipeline m_handle;

			VkPipelineInputAssemblyStateCreateInfo m_input_assembly_state;
			VkPipelineViewportStateCreateInfo m_viewport_state;
			VkPipelineRasterizationStateCreateInfo m_rasterization_state;
			VkPipelineMultisampleStateCreateInfo m_multisample_state;
			VkPipelineDepthStencilStateCreateInfo m_depth_stencil_state;
			std::vector<VkPipelineColorBlendAttachmentState> m_color_blend_attachments;
			VkPipelineColorBlendStateCreateInfo m_color_blend_state;
			VkPipelineDynamicStateCreateInfo m_dynamic_state;
			VkPipelineVertexInputStateCreateInfo m_vertex_input_state;

			std::vector<VkDynamicState> m_dynamic_states;
			std::vector<VkPipelineShaderStageCreateInfo> m_shader_stages;
			std::vector<VkSpecializationInfo> m_specialization_infos;
			std::optional<VkPipelineRenderingCreateInfoKHR> m_rendering_create_info;

			std::vector<VkDescriptorSetLayout> m_descriptor_set_layouts;
			VkPipelineLayout m_layout;

			PipelineType m_type;
		};

	} // namespace graphics

} // namespace hellengine