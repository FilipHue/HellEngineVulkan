#include "hepch.h"
#include "pipeline_manager.h"

namespace hellengine
{

	namespace graphics
	{

		void PipelineManager::Init(VulkanBackend* backend)
		{
			m_backend = backend;
		}

		void PipelineManager::Shutdown()
		{
			for (auto& [name, data] : m_pipeline_data)
			{
				m_backend->DestroyPipeline(data.pipeline);
			}
			m_pipeline_data.clear();
		}

		VulkanPipeline* PipelineManager::CreatePipeline(const std::string& name, const PipelineCreateInfo& create_info, const ShaderStageInfo& shader_info)
		{
			if (m_pipeline_data.find(name) != m_pipeline_data.end())
			{
				HE_GRAPHICS_WARN("Pipeline with name {0} already exists. Returning existing pipeline.", name.c_str());
				return m_pipeline_data[name].pipeline;
			}
			VulkanPipeline* pipeline = m_backend->CreatePipeline(create_info, shader_info);
			m_pipeline_data[name] = { pipeline, create_info };
			return pipeline;
		}

		VulkanPipeline* PipelineManager::GetPipeline(const std::string& name)
		{
			if (m_pipeline_data.find(name) == m_pipeline_data.end())
			{
				HE_GRAPHICS_ERROR("Pipeline with name {0} does not exist.", name.c_str());
				return nullptr;
			}
			return m_pipeline_data[name].pipeline;
		}

	} // namespace graphics

} // namespace hellengine