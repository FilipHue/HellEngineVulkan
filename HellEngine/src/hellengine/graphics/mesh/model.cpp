#include "hepch.h"
#include "model.h"

// Internal
#include <hellengine/graphics/managers/mesh_manager.h>
#include <hellengine/graphics/managers/texture_manager.h>

namespace hellengine
{
	namespace graphics
	{

		Model::Model()
		{
			NO_OP;
		}

		Model::~Model()
		{
			NO_OP;
		}

		void Model::Init(VulkanBackend* backend)
		{
			m_vertex_buffer = nullptr;
			m_index_buffer = nullptr;

			m_textures.push_back(TextureManager::GetInstance()->GetTexture2D(DEFAULT_ERROR_TEXTURE));

			m_backend = MeshManager::GetInstance()->GetBackend();
		}

		void Model::Destroy()
		{
			for (Mesh* mesh : m_meshes)
			{
				delete mesh;
			}

			m_backend->DestroyBuffer(m_vertex_buffer);
			m_backend->DestroyBuffer(m_index_buffer);
			m_backend->DestroyBuffer(m_meshes_buffer);
		}

		template HE_API void Model::UploadToGPU<VertexFormatBase>();
		template HE_API void Model::UploadToGPU<VertexFormatTangent>();
		template<typename VertexT>
		void Model::UploadToGPU()
		{
			/*std::vector<VertexT> final_vertices;
			final_vertices.reserve(m_vertices_raw_data.positions.size());

			for (size_t i = 0; i < m_vertices_raw_data.positions.size(); ++i)
			{
				VertexT vertex{};
				if constexpr (requires { vertex.position; })
				{
					vertex.position = m_vertices_raw_data.positions[i];
				}
				if constexpr (requires { vertex.color; })
				{
					vertex.color = m_vertices_raw_data.colors.has_value() ? m_vertices_raw_data.colors.value()[i] : glm::vec4(1.0f);
				}
				if constexpr (requires { vertex.tex_coord; })
				{
					vertex.tex_coord = m_vertices_raw_data.tex_coords.has_value() ? m_vertices_raw_data.tex_coords.value()[i] : glm::vec2(0.0f);
				}
				if constexpr (requires { vertex.normal; })
				{
					vertex.normal = m_vertices_raw_data.normals.has_value() ? m_vertices_raw_data.normals.value()[i] : glm::vec3(0.0f);
				}
				if constexpr (requires { vertex.tangent; })
				{
					vertex.tangent = m_vertices_raw_data.tangents.has_value() ? m_vertices_raw_data.tangents.value()[i] : glm::vec3(0.0f);
				}
				if constexpr (requires { vertex.bitangent; })
				{
					vertex.bitangent = m_vertices_raw_data.bitangents.has_value() ? m_vertices_raw_data.bitangents.value()[i] : glm::vec3(0.0f);
				}

				final_vertices.push_back(vertex);
			}

			m_vertex_buffer = m_backend->CreateVertexBuffer(final_vertices.data(), static_cast<u32>(final_vertices.size()));
			m_index_buffer = m_backend->CreateIndexBuffer(m_indices.data(), static_cast<u32>(m_indices.size()));*/
		}

		void Model::GenerateModelResources(VulkanPipeline* pipeline, u32 set, TextureType types)
		{
			for (Mesh* mesh : m_meshes)
			{
				MaterialGPUInfo info{};
				info.diffuse_index = mesh->GetMaterialInfo()->Get(TextureType_Diffuse & types);
				info.specular_index = mesh->GetMaterialInfo()->Get(TextureType_Specular & types);
				info.ambient_index = mesh->GetMaterialInfo()->Get(TextureType_Ambient & types);
				info.emissive_index = mesh->GetMaterialInfo()->Get(TextureType_Emissive & types);
				info.height_index = mesh->GetMaterialInfo()->Get(TextureType_Emissive & types);
				info.normal_index = mesh->GetMaterialInfo()->Get(TextureType_Normals & types);
				info.shininess_index = mesh->GetMaterialInfo()->Get(TextureType_Shininess & types);
				info.opacity_index = mesh->GetMaterialInfo()->Get(TextureType_Opacity & types);
				info.displacement_index = mesh->GetMaterialInfo()->Get(TextureType_Displacement & types);
				info.lightmap_index = mesh->GetMaterialInfo()->Get(TextureType_Lightmap & types);
				info.reflection_index = mesh->GetMaterialInfo()->Get(TextureType_Reflection & types);

				m_mesh_gpu_info.push_back(info);
			}

			void* data = m_mesh_gpu_info.data();
			m_meshes_buffer = m_backend->CreateUniformBufferMappedOnce(data, sizeof(MaterialGPUInfo) * (u32)m_mesh_gpu_info.size());

			for (u32 i = 0; i < (u32)m_meshes.size(); i++)
			{
				m_meshes_descriptor.push_back(m_backend->CreateDescriptorSet(pipeline, set));

				DescriptorSetWriteData buffer_data{};
				buffer_data.type = DescriptorType_UniformBuffer;
				buffer_data.binding = 0;
				buffer_data.data.buffer.buffers = new VkBuffer(m_meshes_buffer->GetHandle());
				buffer_data.data.buffer.offsets = new VkDeviceSize(i * sizeof(MaterialGPUInfo));
				buffer_data.data.buffer.ranges = new VkDeviceSize(sizeof(MaterialGPUInfo));

				std::vector<DescriptorSetWriteData> material_writes = { buffer_data };
				m_backend->WriteDescriptor(&m_meshes_descriptor[i], material_writes);
			}

			m_textures_descriptor = m_backend->CreateDescriptorSetVariable(pipeline, set + 1, { (u32)m_textures.size() });

			DescriptorSetWriteData image_data;
			image_data.type = DescriptorType_CombinedImageSampler;
			image_data.binding = 0;
			image_data.data.image.image_views;
			image_data.data.image.samplers;

			std::vector<VkImageView> views;
			std::vector<VkSampler> samplers;
			for (u32 i = 0; i < (u32)m_textures.size(); i++)
			{
				views.push_back(m_textures[i]->GetImageView());
				samplers.push_back(m_textures[i]->GetSampler());
			}
			image_data.data.image.image_views = views.data();
			image_data.data.image.samplers = samplers.data();

			std::vector<DescriptorSetWriteData> texture_writes = { image_data };
			m_backend->WriteDescriptorVariable(&m_textures_descriptor, texture_writes, (u32)m_textures.size(), 0);
		}

		void Model::Draw(VulkanPipeline* pipeline, u32 pipeline_index)
		{
			m_backend->BindVertexBuffer(m_vertex_buffer, 0);
			m_backend->BindIndexBuffer(m_index_buffer, 0);

			m_backend->BindDescriptorSet(pipeline, m_textures_descriptor);

			for (i32 i = 0; i < (i32)m_meshes.size(); i++)
			{
				m_backend->BindDescriptorSet(pipeline, m_meshes_descriptor[i]);
				m_backend->DrawIndexed(m_meshes[i]->GetIndexCount(), 1, m_meshes[i]->GetFirstIndex(), m_meshes[i]->GetVertexStart(), 0);
			}
		}

	} // namespace graphics

} // namespace hellengine