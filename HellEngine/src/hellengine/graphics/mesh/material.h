#pragma once

//Internal
#include <hellengine/math/core.h>
#include <hellengine/graphics/objects/types.h>

namespace hellengine
{

	namespace graphics
	{

		class VulkanTexture2D;
		class VulkanUniformBuffer;

		struct Material
		{
		public:
			Material() = default;
			virtual ~Material() = default;

			virtual void GenerateDescriptorSets(std::vector<DescriptorSetWriteData>& descriptor_data) = 0;
		};

		struct UnlitMaterial : public Material
		{
		public:
			UnlitMaterial();
			~UnlitMaterial();

			void GenerateDescriptorSets(std::vector<DescriptorSetWriteData>& descriptor_data) override;

		public:
			glm::vec4 color;
			VulkanTexture2D* texture_base_color;

		private:
			VulkanUniformBuffer* m_uniform_buffer = nullptr;
		};

		struct LitMaterial : public Material
		{
		public:
			LitMaterial();
			~LitMaterial();

			void GenerateDescriptorSets(std::vector<DescriptorSetWriteData>& descriptor_data) override;

		public:
			glm::vec4 color;
			VulkanTexture2D* texture_base_color;
			VulkanTexture2D* texture_normal;
			VulkanTexture2D* texture_metallic;
			VulkanTexture2D* texture_roughness;
			VulkanTexture2D* texture_ao;

		private:
			VulkanUniformBuffer* m_uniform_buffer = nullptr;
		};

	} // namespace graphics

} // namespace hellengine