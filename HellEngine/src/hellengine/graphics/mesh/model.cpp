#include "hepch.h"
#include "model.h"

// Internal
#include <hellengine/graphics/managers/mesh_manager.h>

namespace hellengine
{
	namespace graphics
	{

		Model::Model()
		{
			m_vertex_buffer = nullptr;
			m_index_buffer = nullptr;

			m_backend = MeshManager::GetInstance()->GetBackend();
		}

		Model::~Model()
		{
			if (m_vertex_buffer)
			{
				m_backend->DestroyBuffer(m_vertex_buffer);
			}

			if (m_index_buffer)
			{
				m_backend->DestroyBuffer(m_index_buffer);
			}

			for (Mesh* mesh : m_meshes)
			{
				delete mesh;
			}
		}

		void Model::GenerateDescriptorSets(VulkanPipeline* pipeline, u32 set)
		{
			for (Mesh* mesh : m_meshes)
			{
				VulkanDescriptorSet* descriptor_set = m_backend->CreateDescriptorSet(pipeline, set);
				m_descriptor_sets.push_back(descriptor_set);

				std::vector<DescriptorSetWriteData> descriptor_data;
				mesh->GetMaterial()->GenerateDescriptorSets(descriptor_data);

				m_backend->WriteDescriptor(&descriptor_set, descriptor_data);
			}
		}

		template HE_API void Model::UploadToGPU<VertexFormatBase>();
		template HE_API void Model::UploadToGPU<VertexFormatTangent>();
		template<typename VertexT>
		void Model::UploadToGPU()
		{
			std::vector<VertexT> final_vertices;
			final_vertices.reserve(m_vertices.size());

			for (size_t i = 0; i < m_vertices.size(); ++i)
			{
				VertexT vertex{};
				if constexpr (requires { vertex.position; })
					vertex.position = m_vertices[i].position;
				if constexpr (requires { vertex.color; })
					vertex.color = m_vertices[i].color;
				if constexpr (requires { vertex.tex_coord; })
					vertex.tex_coord = m_vertices[i].tex_coord;
				if constexpr (requires { vertex.normal; })
					vertex.normal = m_vertices[i].normal;
				if constexpr (requires { vertex.tangent; })
					vertex.tangent = m_vertices[i].tangent;
				if constexpr (requires { vertex.bitangent; })
					vertex.bitangent = m_vertices[i].bitangent;

				final_vertices.push_back(vertex);
			}

			m_vertex_buffer = m_backend->CreateVertexBuffer(final_vertices.data(), static_cast<u32>(final_vertices.size()));
			m_index_buffer = m_backend->CreateIndexBuffer(m_indices.data(), static_cast<u32>(m_indices.size()));
		}

		void Model::Draw(VulkanPipeline* pipeline, u32 pipeline_index)
		{
			m_backend->BindVertexBuffer(m_vertex_buffer, 0);
			m_backend->BindIndexBuffer(m_index_buffer, 0);

			if (m_descriptor_sets.size() && pipeline_index != -1)
			{
				for (u32 i = 0; i < m_meshes.size(); i++)
				{
					m_backend->BindDescriptorSet(pipeline, m_descriptor_sets[i + pipeline_index * m_meshes.size()]);
					m_backend->DrawIndexed(m_meshes[i]->GetIndexCount(), 1, m_meshes[i]->GetFirstIndex(), m_meshes[i]->GetVertexStart(), 0);
				}
			}
			else
			{
				for (u32 i = 0; i < m_meshes.size(); i++)
				{
					m_backend->DrawIndexed(m_meshes[i]->GetIndexCount(), 1, m_meshes[i]->GetFirstIndex(), m_meshes[i]->GetVertexStart(), 0);
				}
			}
		}

	} // namespace graphics

} // namespace hellengine