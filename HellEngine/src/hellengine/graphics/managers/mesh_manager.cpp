#include "hepch.h"
#include "mesh_manager.h"

#include <hellengine/graphics/managers/texture_manager.h>

namespace hellengine
{

	namespace graphics
	{

		void MeshManager::Init(VulkanBackend* backend)
		{
			m_backend = backend;

			m_pools.push_back({
				m_backend->CreateVertexBufferEmpty(MAX_MEMORY_VERTICES),
				m_backend->CreateIndexBufferEmpty(MAX_MEMORY_INDICES),
				0,
				0
				});
			m_current_pool_index = 0;

			m_materials_buffer = m_backend->CreateStorageBufferMappedPersistent(sizeof(MaterialGPUInfo), MAX_MATERIALS);

			m_materials_descriptor = nullptr;
			m_textures_descriptor = nullptr;
		}

		void MeshManager::Shutdown()
		{
			for (auto& pool : m_pools)
			{
				m_backend->DestroyBuffer(pool.vertex_buffer);
				m_backend->DestroyBuffer(pool.index_buffer);
			}
			m_pools.clear();

			for (auto& allocation : m_mesh_allocations)
			{
				delete allocation.mesh;
			}

			m_backend->DestroyBuffer(m_materials_buffer);
		}

		void MeshManager::CreateMeshResource(VulkanPipeline* pipeline, u32 set)
		{
			// Create descriptor set for materials
			{
				m_materials_descriptor = m_backend->CreateDescriptorSet(pipeline, set);

				DescriptorSetWriteData buffer_data{};
				buffer_data.type = DescriptorType_StorageBuffer;
				buffer_data.binding = 0;
				buffer_data.data.buffer.buffers = new VkBuffer(m_materials_buffer->GetHandle());
				buffer_data.data.buffer.offsets = new VkDeviceSize(0);
				buffer_data.data.buffer.ranges = new VkDeviceSize(MAX_MATERIALS * sizeof(MaterialGPUInfo));

				std::vector<DescriptorSetWriteData> material_writes = { buffer_data };
				m_backend->WriteDescriptor(&m_materials_descriptor, material_writes);
			}

			// Create descriptor set for textures
			{
				m_textures_descriptor = m_backend->CreateDescriptorSetVariable(pipeline, set + 1, { 128 });
			}
		}

		template b8 MeshManager::CreateMesh<VertexFormatBase>(std::string name, RawVertexData vertices, std::vector<u32> indices, MaterialInfo* material);
		template<typename VertexT>
		b8 MeshManager::CreateMesh(std::string name, RawVertexData vertices, std::vector<u32> indices, MaterialInfo* material)
		{
			if (m_mesh_allocations_index_map.find(name) != m_mesh_allocations_index_map.end())
			{
				HE_GRAPHICS_WARN("Mesh with name {0} already exists!", name);
				return false;
			}

			u32 vertex_count = (u32)vertices.positions.size();
			u32 index_count = (u32)indices.size();

			if (vertex_count > MAX_VERTICES || index_count > MAX_INDICES)
			{
				HE_GRAPHICS_ERROR("Mesh has more verticies or indices than the maximum allowed!");
				return false;
			}

			BufferPool* current_pool = &m_pools[m_current_pool_index];
			if (current_pool->vertex_count + vertex_count > MAX_VERTICES || current_pool->index_count + index_count > MAX_INDICES)
			{
				HE_GRAPHICS_ERROR("Exceeded maximum vertex or index capacity in the current pool!");

				CreatePool();
				current_pool = &m_pools[m_current_pool_index];
			}

			std::vector<VertexT> final_vertices;
			final_vertices.reserve(vertices.positions.size());

			for (size_t i = 0; i < vertices.positions.size(); ++i)
			{
				VertexT vertex{};
				if constexpr (requires { vertex.position; })
				{
					vertex.position = vertices.positions[i];
				}
				if constexpr (requires { vertex.color; })
				{
					vertex.color = vertices.colors.size() ? vertices.colors[i] : glm::vec4(1.0f);
				}
				if constexpr (requires { vertex.tex_coord; })
				{
					vertex.tex_coord = vertices.tex_coords.size() ? vertices.tex_coords[i] : glm::vec2(0.0f);
				}
				if constexpr (requires { vertex.normal; })
				{
					vertex.normal = vertices.normals.size() ? vertices.normals[i] : glm::vec3(0.0f);
				}
				if constexpr (requires { vertex.tangent; })
				{
					vertex.tangent = vertices.tangents.size() ? vertices.tangents[i] : glm::vec3(0.0f);
				}
				if constexpr (requires { vertex.bitangent; })
				{
					vertex.bitangent = vertices.bitangents.size() ? vertices.bitangents[i] : glm::vec3(0.0f);
				}

				final_vertices.push_back(vertex);
			}

			m_backend->UpdateVertexBuffer(current_pool->vertex_buffer, current_pool->vertex_count * sizeof(VertexT), final_vertices.data(), vertex_count * sizeof(VertexT));
			m_backend->UpdateIndexBuffer(current_pool->index_buffer, current_pool->index_count * sizeof(u32), indices.data(), index_count * sizeof(u32));

			Mesh* mesh = new Mesh();
			mesh->SetName(name.c_str());
			mesh->SetFirstIndex(current_pool->index_count);
			mesh->SetVertexStart(current_pool->vertex_count);
			mesh->SetIndexCount(index_count);
			mesh->SetRawData(vertices);
			mesh->SetMaterialInfo(material);

			MeshAllocation allocation = { mesh, m_current_pool_index };
			m_mesh_allocations.push_back(allocation);
			m_mesh_allocations_index_map[name] = m_mesh_allocations.size() - 1;

			current_pool->vertex_count += vertex_count;
			current_pool->index_count += index_count;

			current_pool->meshes.push_back(mesh);

			return true;
		}

		void MeshManager::UploadToGpu(VulkanPipeline* pipeline, u32 set, TextureType types)
		{
			if (m_materials_descriptor == nullptr || m_textures_descriptor == nullptr)
			{
				HE_GRAPHICS_ERROR("MeshManager::UploadToGpu called before creating descriptor sets!");
				return;
			}

			if (m_mesh_allocations.size() > MAX_MATERIALS)
			{
				HE_GRAPHICS_ERROR("Exceeded maximum number of materials in MeshManager!");
				return;
			}

			if (m_last_mesh_index == m_mesh_allocations.size())
			{
				return;
			}

			for (u32 i = m_last_mesh_index; i < m_mesh_allocations.size(); i++)
			{
				MaterialInfo* material_info = m_mesh_allocations[i].mesh->GetMaterialInfo();

				MaterialGPUInfo info{};
				info.diffuse_index = material_info->Get(TextureType_Diffuse & types);
				info.specular_index = material_info->Get(TextureType_Specular & types);
				info.ambient_index = material_info->Get(TextureType_Ambient & types);
				info.emissive_index = material_info->Get(TextureType_Emissive & types);
				info.height_index = material_info->Get(TextureType_Height & types);
				info.normal_index = material_info->Get(TextureType_Normals & types);
				info.shininess_index = material_info->Get(TextureType_Shininess & types);
				info.opacity_index = material_info->Get(TextureType_Opacity & types);
				info.displacement_index = material_info->Get(TextureType_Displacement & types);
				info.lightmap_index = material_info->Get(TextureType_Lightmap & types);
				info.reflection_index = material_info->Get(TextureType_Reflection & types);

				m_mesh_gpu_info.push_back(info);
			}

			void* data = &m_mesh_gpu_info[m_last_mesh_index];
			m_backend->UpdateStorageBuffer(m_materials_buffer, data, sizeof(MaterialGPUInfo) * ((u32)m_mesh_gpu_info.size() - m_last_mesh_index), sizeof(MaterialGPUInfo) * m_last_mesh_index);
			m_last_mesh_index = (u32)m_mesh_gpu_info.size();

			DescriptorSetWriteData image_data;
			image_data.type = DescriptorType_CombinedImageSampler;
			image_data.binding = 0;
			image_data.data.image.image_views;
			image_data.data.image.samplers;

			std::vector<VkImageView> views;
			std::vector<VkSampler> samplers;
			for (auto& texture : TextureManager::GetInstance()->m_textures_2d_vector)
			{
				if (texture == nullptr)
				{
					continue;
				}
				views.push_back(texture->GetImageView());
				samplers.push_back(texture->GetSampler());
			}
			image_data.data.image.image_views = views.data();
			image_data.data.image.samplers = samplers.data();

			std::vector<DescriptorSetWriteData> texture_writes = { image_data };
			m_backend->WriteDescriptorVariable(&m_textures_descriptor, texture_writes, (u32)TextureManager::GetInstance()->m_textures_2d_vector.size(), 0);
		}

		void MeshManager::DrawMeshes(VulkanPipeline* pipeline)
		{
			if (m_pools.empty() || m_materials_descriptor == nullptr || m_textures_descriptor == nullptr)
			{
				return;
			}

			m_backend->BindDescriptorSet(pipeline, m_materials_descriptor);
			m_backend->BindDescriptorSet(pipeline, m_textures_descriptor);

			u32 offset = 0;
			for (const auto& pool : m_pools)
			{
				m_backend->BindVertexBuffer(pool.vertex_buffer, 0);
				m_backend->BindIndexBuffer(pool.index_buffer, 0);

				u32 index = 0;
				for (u32 i = 0; i < (u32)pool.meshes.size(); i++)
				{
					u32 material_index = index + offset;
					m_backend->BindPushConstants(pipeline, ShaderStage_Fragment, 0, sizeof(i32), &material_index);
					m_backend->DrawIndexed(pool.meshes[i]->GetIndexCount(), 1, pool.meshes[i]->GetFirstIndex(), pool.meshes[i]->GetVertexStart(), 0);

					index++;
				}

				offset += (u32)pool.meshes.size();
			}
		}

		MeshManager* MeshManager::GetInstance()
		{
			static MeshManager instance;
			return &instance;
		}

		void MeshManager::CreatePool()
		{
			BufferPool pool;

			pool.vertex_buffer = m_backend->CreateVertexBufferEmpty(MAX_MEMORY_VERTICES);
			pool.index_buffer = m_backend->CreateIndexBufferEmpty(MAX_MEMORY_INDICES);
			pool.vertex_count = 0;
			pool.index_count = 0;

			m_pools.push_back(pool);
			m_current_pool_index = (u32)m_pools.size() - 1;
		}

	} // namespace graphics

} // namespace hellengine