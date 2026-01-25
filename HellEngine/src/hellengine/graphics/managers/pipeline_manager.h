#pragma once

// Internal
#include <hellengine/graphics/backend/vulkan_backend.h>
#include <hellengine/graphics/objects/vulkan_pipeline.h>

namespace hellengine
{

	namespace graphics
	{

		class PipelineManager : public Singleton<PipelineManager>
		{
		public:

			void Init(VulkanBackend* backend);
			void Shutdown();

			VulkanPipeline* CreatePipeline(const std::string& name, const PipelineCreateInfo& create_info, const ShaderStageInfo& shader_info);
			VulkanPipeline* GetPipeline(const std::string& name);

		private:
			struct PipelineData
			{
				VulkanPipeline* pipeline;
				PipelineCreateInfo create_info;
			};
			std::unordered_map<std::string, PipelineData> m_pipeline_data;

			VulkanBackend* m_backend;
		};

	} // namespace graphics

} // namespace hellengine