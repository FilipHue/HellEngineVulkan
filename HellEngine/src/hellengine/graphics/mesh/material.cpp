#include "hepch.h"
#include "material.h"

// Internal
#include <hellengine/graphics/objects/vulkan_texture.h>
#include <hellengine/graphics/objects/vulkan_buffer.h>

#include <hellengine/graphics/managers/material_manager.h>

namespace hellengine
{

	namespace graphics
	{

		UnlitMaterial::UnlitMaterial() : Material(), color(0.0f), texture_base_color(nullptr)
		{
			m_uniform_buffer = MaterialManager::GetInstance()->GetBackend()->CreateUniformBufferMappedPersistent(sizeof(color));
		}

		UnlitMaterial::~UnlitMaterial()
		{
			MaterialManager::GetInstance()->GetBackend()->DestroyUniformBuffer(m_uniform_buffer);
		}

		void UnlitMaterial::GenerateDescriptorSets(std::vector<DescriptorSetWriteData>& descriptor_data)
		{
			MaterialManager::GetInstance()->GetBackend()->UpdateUniformBufferOnce(m_uniform_buffer, &color, sizeof(color));

			DescriptorSetWriteData descriptor_data0;
			descriptor_data0.type = DescriptorType_UniformBuffer;
			descriptor_data0.binding = 0;
			descriptor_data0.data.buffer.buffer = m_uniform_buffer->GetHandle();
			descriptor_data0.data.buffer.offset = 0;
			descriptor_data0.data.buffer.range = sizeof(color);
			descriptor_data.push_back(descriptor_data0);

			if (texture_base_color)
			{
				DescriptorSetWriteData descriptor_data1;
				descriptor_data1.type = DescriptorType_CombinedImageSampler;
				descriptor_data1.binding = 1;
				descriptor_data1.data.image.image_view = texture_base_color->GetImageView();
				descriptor_data1.data.image.sampler = texture_base_color->GetSampler();
				descriptor_data.push_back(descriptor_data1);
			}
		}

        LitMaterial::LitMaterial() : Material(), color(0.0f), texture_base_color(nullptr), texture_normal(nullptr), texture_metallic(nullptr), texture_roughness(nullptr), texture_ao(nullptr)
        {
            m_uniform_buffer = MaterialManager::GetInstance()->GetBackend()->CreateUniformBufferMappedPersistent(sizeof(color));
        }

		LitMaterial::~LitMaterial()
		{
			MaterialManager::GetInstance()->GetBackend()->DestroyUniformBuffer(m_uniform_buffer);
		}

		void LitMaterial::GenerateDescriptorSets(std::vector<DescriptorSetWriteData>& descriptor_data)
		{
			MaterialManager::GetInstance()->GetBackend()->UpdateUniformBufferOnce(m_uniform_buffer, &color, sizeof(color));

			DescriptorSetWriteData descriptor_data0;
			descriptor_data0.type = DescriptorType_UniformBuffer;
			descriptor_data0.binding = 0;
			descriptor_data0.data.buffer.buffer = m_uniform_buffer->GetHandle();
			descriptor_data0.data.buffer.offset = 0;
			descriptor_data0.data.buffer.range = sizeof(color);
			descriptor_data.push_back(descriptor_data0);

			if (texture_base_color)
			{
				DescriptorSetWriteData descriptor_data1;
				descriptor_data1.type = DescriptorType_CombinedImageSampler;
				descriptor_data1.binding = 1;
				descriptor_data1.data.image.image_view = texture_base_color->GetImageView();
				descriptor_data1.data.image.sampler = texture_base_color->GetSampler();
				descriptor_data.push_back(descriptor_data1);
			}

			if (texture_normal)
			{
				DescriptorSetWriteData descriptor_data2;
				descriptor_data2.type = DescriptorType_CombinedImageSampler;
				descriptor_data2.binding = 2;
				descriptor_data2.data.image.image_view = texture_normal->GetImageView();
				descriptor_data2.data.image.sampler = texture_normal->GetSampler();
				descriptor_data.push_back(descriptor_data2);
			}

			if (texture_metallic)
			{
				DescriptorSetWriteData descriptor_data3;
				descriptor_data3.type = DescriptorType_CombinedImageSampler;
				descriptor_data3.binding = 3;
				descriptor_data3.data.image.image_view = texture_metallic->GetImageView();
				descriptor_data3.data.image.sampler = texture_metallic->GetSampler();
				descriptor_data.push_back(descriptor_data3);
			}

			if (texture_roughness)
			{
				DescriptorSetWriteData descriptor_data4;
				descriptor_data4.type = DescriptorType_CombinedImageSampler;
				descriptor_data4.binding = 4;
				descriptor_data4.data.image.image_view = texture_roughness->GetImageView();
				descriptor_data4.data.image.sampler = texture_roughness->GetSampler();
				descriptor_data.push_back(descriptor_data4);
			}

			if (texture_ao)
			{
				DescriptorSetWriteData descriptor_data5;
				descriptor_data5.type = DescriptorType_CombinedImageSampler;
				descriptor_data5.binding = 5;
				descriptor_data5.data.image.image_view = texture_ao->GetImageView();
				descriptor_data5.data.image.sampler = texture_ao->GetSampler();
				descriptor_data.push_back(descriptor_data5);
			}
		}

	} // namespace graphics

} // namespace hellengine