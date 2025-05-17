#pragma once

// Internal
#include "shared.h"
#include <hellengine/resources/file_manager.h>

namespace hellengine
{

	using namespace resources;
	namespace graphics
	{
		
		class VulkanShader
		{
		public:
			VulkanShader();
			~VulkanShader();

			void Create(const VulkanInstance& instance, const VulkanDevice& device, const std::string& path, ShaderType type);
			void Create(const VulkanInstance& instance, const VulkanDevice& device, File& file, ShaderType type);
			void Destroy(const VulkanInstance& instance, const VulkanDevice& device) const;

			VkShaderModule GetHandle() const { return m_handle; }
			ShaderType GetType() const { return m_type; }

			void SetType(ShaderType type) { m_type = type; }

			static VkPipelineShaderStageCreateInfo GetShaderStageCreateInfo(const VulkanShader& shader);
			static void DestroyShaderStageCreateInfo(const VulkanInstance& instance, const VulkanDevice& device, VkPipelineShaderStageCreateInfo& shader_stage_info);

		private:
			VkShaderModule m_handle;

			ShaderType m_type;
		};

		static std::vector<char> ReadSPIRVSource(const char* path);
		static std::vector<char> CompileSource(File& file, ShaderType type);

	} // namespace graphics

} // namespace hellengine