#include "hepch.h"
#include "mesh_manager.h"

namespace hellengine
{

	namespace graphics
	{

		void MeshManager::Init(VulkanBackend* backend)
		{
			m_backend = backend;

			m_global_vertex_buffer = m_backend->CreateVertexBufferEmpty(MAX_MEMORY_VERTICES);
			m_global_index_buffer = m_backend->CreateIndexBufferEmpty(MAX_MEMORY_INDICES);

			m_global_vertex_count = 0;
			m_global_index_count = 0;
		}

		void MeshManager::Shutdown()
		{
			m_backend->DestroyBuffer(m_global_vertex_buffer);
			m_backend->DestroyBuffer(m_global_index_buffer);
		}

		b8 MeshManager::CreateMesh(std::string name, std::vector<VertexFormatBase> vertices, std::vector<u32> indices)
		{
			if (m_meshes.find(name) != m_meshes.end())
			{
				HE_GRAPHICS_WARN("Mesh with name {0} already exists!", name);
				return false;
			}

			if (m_global_vertex_count >= MAX_VERTICES)
			{
				HE_GRAPHICS_ERROR("Exceeded maximum vertex capacity!");
				return false;
			}

			if (m_global_index_count >= MAX_INDICES)
			{
				HE_GRAPHICS_ERROR("Exceeded maximum index capacity!");
				return false;
			}

			u32 max_vertex_count = MIN((u32)vertices.size(), MAX_VERTICES - m_global_vertex_count);
			u32 max_index_count = MIN((u32)indices.size(), MAX_INDICES - m_global_index_count);

			Mesh* mesh = new Mesh();
			mesh->SetFirstIndex(m_global_index_count);
			mesh->SetVertexStart(m_global_vertex_count);
			mesh->SetIndexCount((u32)indices.size());

			m_backend->UpdateVertexBuffer(m_global_vertex_buffer, m_global_vertex_count * sizeof(VertexFormatBase), vertices.data(), max_vertex_count * sizeof(VertexFormatBase));
			m_backend->UpdateIndexBuffer(m_global_index_buffer, m_global_index_count * sizeof(u32), indices.data(), max_index_count * sizeof(u32));

			m_global_vertex_count += max_vertex_count;
			m_global_index_count += max_index_count;

			m_meshes[name] = mesh;

			return true;
		}

		void MeshManager::DrawMesh(std::string name, u32 instance_count)
		{
			if (m_meshes.find(name) == m_meshes.end())
			{
				HE_GRAPHICS_ERROR("Mesh with name {0} does not exist!", name);
				return;
			}

			Mesh* mesh = m_meshes[name];

			m_backend->BindVertexBuffer(m_global_vertex_buffer, mesh->GetVertexStart() * sizeof(VertexFormatBase));
			m_backend->BindIndexBuffer(m_global_index_buffer, mesh->GetFirstIndex() * sizeof(u32));

			m_backend->DrawIndexed(mesh->GetIndexCount(), instance_count, 0, 0, 0);
		}

		MeshManager* MeshManager::GetInstance()
		{
			static MeshManager instance;
			return &instance;
		}

	} // namespace graphics

} // namespace hellengine