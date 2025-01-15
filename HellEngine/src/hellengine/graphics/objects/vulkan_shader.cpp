#include "hepch.h"
#include "vulkan_shader.h"

// Internal
#include "vulkan_instance.h"
#include "vulkan_device.h"

namespace hellengine
{

	namespace graphics
	{

		VulkanShader::VulkanShader()
		{
			m_handle = VK_NULL_HANDLE;

			m_type = ShaderType_Undefined;
		}

		VulkanShader::~VulkanShader()
		{
			NO_OP;
		}

		void VulkanShader::Create(const VulkanInstance& instance, const VulkanDevice& device, const std::string& path, ShaderType type)
		{
			m_type = type;

			std::vector<char> code = ReadSPIRVSource(path.data());

			VkShaderModuleCreateInfo create_info = {};
			create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			create_info.codeSize = code.size();
			create_info.pCode = reinterpret_cast<const u32*>(code.data());

			VK_CHECK(vkCreateShaderModule(device.GetLogicalDevice(), &create_info, instance.GetAllocator(), &m_handle));
		}

		void VulkanShader::Create(const VulkanInstance& instance, const VulkanDevice& device, resources::File& file, ShaderType type)
		{
			m_type = type;

			std::vector<char> code = CompileSource(file, type);

			VkShaderModuleCreateInfo create_info = {};
			create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			create_info.codeSize = code.size();
			create_info.pCode = reinterpret_cast<const u32*>(code.data());

			VK_CHECK(vkCreateShaderModule(device.GetLogicalDevice(), &create_info, instance.GetAllocator(), &m_handle));
		}

		void VulkanShader::Destroy(const VulkanInstance& instance, const VulkanDevice& device) const
		{
			vkDestroyShaderModule(device.GetLogicalDevice(), m_handle, instance.GetAllocator());
		}

		VkPipelineShaderStageCreateInfo VulkanShader::GetShaderStageCreateInfo(const VulkanShader& shader)
		{
			VkPipelineShaderStageCreateInfo shader_stage_info = {};
			shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			shader_stage_info.pNext = VK_NULL_HANDLE;
			shader_stage_info.flags = 0;
			shader_stage_info.stage = ShaderTypeToVulkan(shader.GetType());
			shader_stage_info.module = shader.GetHandle();
			shader_stage_info.pName = "main";
			shader_stage_info.pSpecializationInfo = VK_NULL_HANDLE;

			return shader_stage_info;
		}

		void VulkanShader::DestroyShaderStageCreateInfo(const VulkanInstance& instance, const VulkanDevice& device, VkPipelineShaderStageCreateInfo& shader_stage_info)
		{
			vkDestroyShaderModule(device.GetLogicalDevice(), shader_stage_info.module, instance.GetAllocator());
		}

		std::vector<char> ReadSPIRVSource(const char* path)
		{
			std::ifstream file(path, std::ios::ate | std::ios::binary);

			if (!file.is_open()) {
				HE_CORE_ERROR("Failed to open file: {0}", path);
			}

			size_t file_size = (size_t)file.tellg();
			std::vector<char> buffer(file_size);

			file.seekg(0);
			file.read(buffer.data(), file_size);
			file.close();

			return buffer;
		}

		std::vector<char> CompileSource(resources::File& file, ShaderType type)
		{
			std::string binary_name = type == ShaderType_Vertex ? "vert" : "frag";
			binary_name = file.GetAbsoluteDirectory() + "\\" + binary_name + ".spv";

			std::string command = "C:/VulkanSDK/1.3.290.0/Bin/glslc.exe " + file.GetAbsolutePath() + " -o " + binary_name;
			system(command.c_str());

			return ReadSPIRVSource(binary_name.c_str());
		}

	} // namespace graphics

} // namespace hellengine