#include "hepch.h"
#include "mesh_manager.h"

#include <hellengine/math/hash.h>
#include <hellengine/graphics/managers/texture_manager.h>
#include <hellengine/ecs/scene/scene_manager.h>

namespace hellengine
{
	using namespace ecs;
	using namespace math;

	namespace graphics
	{

		void MeshManager::Init(VulkanBackend* backend)
		{
			m_backend = backend;

			m_pools.clear();
			m_pools.push_back({
				m_backend->CreateVertexBufferEmpty(MAX_MEMORY_VERTICES),
				m_backend->CreateIndexBufferEmpty(MAX_MEMORY_INDICES),
				0,
				0
				});

			m_materials_buffer = m_backend->CreateStorageBufferMappedPersistent(sizeof(MaterialGPUInfo), MAX_MATERIALS);
			m_materials_descriptor = nullptr;
			m_textures_descriptor = nullptr;
			m_last_texture_index = 0;

			m_draw_commands_buffer = nullptr;
			m_per_draw_data_buffer = nullptr;
			m_per_draw_data_descriptor = nullptr;

			m_mesh_instances_allocation.clear();
			m_mesh_instance_allocation_map.clear();

			m_mesh_gpu_info.clear();
			m_draw_commands.clear();
			m_per_draw_data.clear();

			m_descriptor_image_views.clear();
			m_descriptor_image_samplers.clear();
		}

		void MeshManager::Shutdown()
		{
			CleanUp();

			for (auto& pool : m_pools)
			{
				m_backend->DestroyBuffer(pool.vertex_buffer);
				m_backend->DestroyBuffer(pool.index_buffer);

				pool.mesh_list.clear();
			}
			m_pools.clear();

			m_mesh_instances_allocation.clear();
			m_mesh_instance_allocation_map.clear();

			if (m_materials_buffer)
			{
				m_backend->DestroyBuffer(m_materials_buffer);
				m_materials_buffer = nullptr;
			}

			if (m_draw_commands_buffer)
			{
				m_backend->DestroyBuffer(m_draw_commands_buffer);
				m_draw_commands_buffer = nullptr;
			}

			if (m_per_draw_data_buffer)
			{
				m_backend->DestroyBuffer(m_per_draw_data_buffer);
				m_per_draw_data_buffer = nullptr;
			}
		}

		void MeshManager::CreateDescriptors(VulkanPipeline* pipeline, u32 set)
		{
			// Create descriptor set for materials
			{
				m_materials_descriptor = m_backend->CreateDescriptorSet(pipeline, SHADER_MATERIALS_BINDING);

				DescriptorSetWriteData buffer_data{};
				buffer_data.type = DescriptorType_StorageBuffer;
				buffer_data.binding = 0;

				VkBuffer buffer_handle = m_materials_buffer->GetHandle();
				VkDeviceSize offset = 0;
				VkDeviceSize range = MAX_MATERIALS * sizeof(MaterialGPUInfo);

				buffer_data.data.buffer.buffers = &buffer_handle;
				buffer_data.data.buffer.offsets = &offset;
				buffer_data.data.buffer.ranges = &range;

				std::vector<DescriptorSetWriteData> material_writes = { buffer_data };
				m_backend->WriteDescriptor(&m_materials_descriptor, material_writes);
			}

			// Create descriptor set for per-draw data
			{
				m_per_draw_data_descriptor = m_backend->CreateDescriptorSet(pipeline, SHADER_PER_DRAW_DATA_BINDING);
			}

			// Create descriptor set for textures (variable count)
			{
				m_textures_descriptor = m_backend->CreateDescriptorSetVariable(pipeline, SHADER_TEXTURES_BINDING, { MAX_TEXTURES });
			}
		}

		Mesh* MeshManager::CreateMesh(std::string name, RawVertexData vertices, std::vector<u32> indices, MaterialInfo* material)
		{
			MeshHash hash = ComputeMeshHash(vertices, indices);
			auto git = m_meshes_buffer_map.find(hash);
			if (git != m_meshes_buffer_map.end())
			{
				HE_GRAPHICS_WARN("Mesh geometry already exists, returning existing mesh.");
				return m_meshes[git->second.mesh_index];
			}

			Mesh* mesh = new Mesh();

			mesh->SetName(name);
			mesh->SetRawData(vertices);
			mesh->SetIndices(indices);
			mesh->SetMaterialInfo(material);

			return mesh;
		}

		void MeshManager::CreateMeshInstance(UUID id, Mesh* mesh, MaterialInfo* material)
		{
			MeshInstanceAllocation* allocation = new MeshInstanceAllocation();

			MeshHash hash = ComputeMeshHash(mesh->GetRawData(), mesh->GetIndices());
			auto git = m_meshes_buffer_map.find(hash);
			if (git == m_meshes_buffer_map.end())
			{
				HE_GRAPHICS_ERROR("Mesh geometry not uploaded before creating instance!");
				return;
			}

			u32 mesh_index = 0;
			auto it = std::find(m_meshes.begin(), m_meshes.end(), mesh);
			if (it != m_meshes.end())
			{
				mesh_index = std::distance(m_meshes.begin(), it);
			}

			allocation->material_info = material != nullptr ? material : mesh->GetMaterialInfo();
			allocation->mesh_index = mesh_index;
			allocation->entity_id = id;
			allocation->buffer_allocation = &git->second;

			m_meshes_buffer_map[hash].ref_count++;

			m_mesh_instances_allocation.push_back(allocation);
			m_mesh_instance_allocation_map[id] = (u32)(m_mesh_instances_allocation.size() - 1);
		}

		void MeshManager::RemoveMeshInstance(UUID id)
		{
			auto it = m_mesh_instance_allocation_map.find(id);
			if (it == m_mesh_instance_allocation_map.end())
			{
				HE_GRAPHICS_WARN("Mesh instance with given UUID not found, cannot remove instance.");
				return;
			}
			u32 allocation_index = it->second;
			MeshInstanceAllocation* allocation = m_mesh_instances_allocation[allocation_index];
			Mesh* mesh = m_meshes[allocation->mesh_index];
			MeshHash hash = ComputeMeshHash(mesh->GetRawData(), mesh->GetIndices());
			auto git = m_meshes_buffer_map.find(hash);
			if (git != m_meshes_buffer_map.end())
			{
				if (git->second.ref_count > 0)
				{
					git->second.ref_count--;
				}
			}
			delete allocation;
			m_mesh_instances_allocation.erase(m_mesh_instances_allocation.begin() + allocation_index);
			m_mesh_instance_allocation_map.erase(it);
			// Update indices in map
			for (u32 i = allocation_index; i < m_mesh_instances_allocation.size(); i++)
			{
				m_mesh_instance_allocation_map[m_mesh_instances_allocation[i]->entity_id] = i;
			}

			CreateDrawCommands();
			CreatePackedData();
		}

		template b8 MeshManager::UploadMeshGeometry<VertexFormatBase>(Mesh* mesh);
		template b8 MeshManager::UploadMeshGeometry<VertexFormatTangent>(Mesh* mesh);

		template<typename VertexT>
		b8 MeshManager::UploadMeshGeometry(Mesh* mesh)
		{
			RawVertexData& vertices = mesh->GetRawData();
			std::vector<u32>& indices = mesh->GetIndices();

			const u32 vertex_count = (u32)vertices.positions.size();
			const u32 index_count = (u32)indices.size();

			if (vertex_count > MAX_VERTICES || index_count > MAX_INDICES)
			{
				HE_GRAPHICS_ERROR(
					"Mesh has more verticies or indices than the maximum allowed per pool!\n\tMesh has {0} verticies and {1} indices\n\tMaximum allowed is {2} vertices and {3} indices",
					vertex_count, index_count, MAX_VERTICES, MAX_INDICES);

				return false;
			}

			MeshHash hash = ComputeMeshHash(vertices, indices);

			if (m_meshes_buffer_map.find(hash) != m_meshes_buffer_map.end())
			{
				HE_GRAPHICS_WARN("Mesh geometry already uploaded, skipping upload.");

				return true;
			}

			BufferPool* current_pool = nullptr;
			u32 current_pool_index = 0;
			for (auto& pool : m_pools)
			{
				if (pool.vertex_count + vertex_count <= MAX_VERTICES && pool.index_count + index_count <= MAX_INDICES)
				{
					current_pool = &pool;
					break;
				}

				current_pool_index++;
			}

			if (!current_pool)
			{
				HE_GRAPHICS_WARN("No existing buffer pool has enough space, creating a new pool.");

				CreatePool();
				current_pool = &m_pools.back();
				current_pool_index = (u32)(m_pools.size() - 1);
			}

			m_meshes.push_back(mesh);
			current_pool->mesh_list.push_back((u32)m_meshes.size() - 1);

			std::vector<VertexT> final_vertices;
			final_vertices.reserve(vertices.positions.size());

			for (size_t i = 0; i < vertices.positions.size(); ++i)
			{
				VertexT vtx{};
				if constexpr (requires { vtx.position;	}) vtx.position = vertices.positions[i];
				if constexpr (requires { vtx.color;		}) vtx.color = vertices.colors.size() ? vertices.colors[i] : glm::vec4(1.0f);
				if constexpr (requires { vtx.tex_coord; }) vtx.tex_coord = vertices.tex_coords.size() ? vertices.tex_coords[i] : glm::vec2(0.0f);
				if constexpr (requires { vtx.normal;	}) vtx.normal = vertices.normals.size() ? vertices.normals[i] : glm::vec3(0.0f);
				if constexpr (requires { vtx.tangent;	}) vtx.tangent = vertices.tangents.size() ? vertices.tangents[i] : glm::vec3(0.0f);
				if constexpr (requires { vtx.bitangent; }) vtx.bitangent = vertices.bitangents.size() ? vertices.bitangents[i] : glm::vec3(0.0f);

				final_vertices.push_back(vtx);
			}

			m_backend->UpdateVertexBuffer(current_pool->vertex_buffer, current_pool->vertex_count * sizeof(VertexT), final_vertices.data(), vertex_count * sizeof(VertexT));
			m_backend->UpdateIndexBuffer(current_pool->index_buffer, current_pool->index_count * sizeof(u32), indices.data(), index_count * sizeof(u32));

			MeshBufferAllocation alloc{};

			alloc.index_count = index_count;
			alloc.index_offset = current_pool->index_count;
			alloc.vertex_count = vertex_count;
			alloc.vertex_offset = current_pool->vertex_count;
			alloc.pool_index = current_pool_index;
			alloc.ref_count = 0;
			alloc.mesh_index = (u32)m_meshes.size() - 1;

			m_meshes_buffer_map[hash] = alloc;

			current_pool->vertex_count += vertex_count;
			current_pool->index_count += index_count;

			return true;
		}

		void MeshManager::SetMeshInstanceFilter(UUID id, Mesh* mesh)
		{
			auto it = m_mesh_instance_allocation_map.find(id);
			if (it == m_mesh_instance_allocation_map.end())
			{
				HE_GRAPHICS_DEBUG("Mesh instance with given UUID not found, creating new instance.");
				CreateMeshInstance(id, mesh);
				it = m_mesh_instance_allocation_map.find(id);
			}
			u32 allocation_index = it->second;
			MeshInstanceAllocation* allocation = m_mesh_instances_allocation[allocation_index];

			if (mesh == nullptr)
			{
				HE_GRAPHICS_ERROR("Cannot set instance filter to null mesh!");
				return;
			}

			MeshHash hash = ComputeMeshHash(mesh->GetRawData(), mesh->GetIndices());
			auto git = m_meshes_buffer_map.find(hash);
			if (git == m_meshes_buffer_map.end())
			{
				HE_GRAPHICS_ERROR("Mesh geometry not uploaded before setting instance filter!");
				return;
			}
			allocation->mesh_index = git->second.mesh_index;
			allocation->buffer_allocation = &git->second;
			CreateDrawCommands();
			CreatePackedData();
		}

		void MeshManager::UploadToGpu(TextureType types)
		{
			if (m_materials_descriptor == nullptr || m_textures_descriptor == nullptr)
			{
				HE_GRAPHICS_ERROR("Cannot upload to GPU before creating descriptor sets!");
				return;
			}

			if (!(m_last_mesh_index == m_meshes.size()))
			{
				for (u32 i = m_last_mesh_index; i < m_meshes.size(); i++)
				{
					MaterialInfo* material_info = m_meshes[i]->GetMaterialInfo();

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
			}

			{
				const u32 texCount = (u32)TextureManager::GetInstance()->m_textures_2d_vector.size();
				if (texCount > 0 && texCount != m_last_texture_index)
				{
					m_descriptor_image_views.clear();
					m_descriptor_image_samplers.clear();
					m_descriptor_image_views.reserve(texCount);
					m_descriptor_image_samplers.reserve(texCount);

					for (u32 i = 0; i < texCount; ++i)
					{
						auto texture_opt = TextureManager::GetInstance()->m_textures_2d_vector.at(i);
						if (texture_opt.has_value())
						{
							VulkanTexture2D* texture = texture_opt.value();
							m_descriptor_image_views.push_back(texture->GetImageView());
							m_descriptor_image_samplers.push_back(texture->GetSampler());
						}
						else
						{
							// If you need stable indexing with holes, use a fallback texture here.
							// For now, we skip missing entries.
						}
					}

					if (!m_descriptor_image_views.empty())
					{
						DescriptorSetWriteData image_data{};
						image_data.type = DescriptorType_CombinedImageSampler;
						image_data.binding = 0;
						image_data.data.image.image_views = m_descriptor_image_views.data();
						image_data.data.image.samplers = m_descriptor_image_samplers.data();

						std::vector<DescriptorSetWriteData> writes = { image_data };

						m_backend->WriteDescriptorVariable(
							&m_textures_descriptor,
							writes,
							(u32)m_descriptor_image_views.size(),
							0
						);
					}

					m_last_texture_index = texCount;
				}
			}

			CreateDrawCommands();
			CreatePackedData();
			UpdatePerDrawData();
		}

		void MeshManager::CreateDrawCommands()
		{
			m_draw_commands.clear();

			// Sort mesh instances by pools
			std::sort(m_mesh_instances_allocation.begin(), m_mesh_instances_allocation.end(), [](MeshInstanceAllocation* a, MeshInstanceAllocation* b)
				{
					return a->buffer_allocation->pool_index < b->buffer_allocation->pool_index;
				});

			// Create Draw commands after pool order
			u32 current_pool = 0;
			u32 current_instance = 0;
			u32 draw_id = 0;
			while (current_pool < (u32)m_pools.size())
			{
				while (current_instance < (u32)m_mesh_instances_allocation.size())
				{
					MeshInstanceAllocation* instance = m_mesh_instances_allocation[current_instance];
					if (current_pool != instance->buffer_allocation->pool_index)
					{
						break;
					}

					VkDrawIndexedIndirectCommand command{};
					command.indexCount = instance->buffer_allocation->index_count;
					command.instanceCount = 1;
					command.firstIndex = instance->buffer_allocation->index_offset;
					command.vertexOffset = instance->buffer_allocation->vertex_offset;
					command.firstInstance = draw_id;

					m_draw_commands.push_back(command);

					current_instance++;
					draw_id += 1;
				}
				current_pool++;
			}

			if (m_draw_commands.empty())
			{
				HE_GRAPHICS_WARN("No draw commands to create buffers for!");
				return;
			}

			const u32 draw_count = (u32)m_draw_commands.size();

			if (m_draw_commands_buffer)
			{
				VulkanBuffer* oldBuffer = m_draw_commands_buffer;
				m_deletion_queue.PushFunction([backend = m_backend, oldBuffer]() {
					backend->DestroyBuffer(oldBuffer);
					});
			}

			m_draw_commands_buffer = m_backend->CreateDrawIndirectBuffer(
				sizeof(VkDrawIndexedIndirectCommand),
				draw_count
			);

			m_backend->UpdateDrawIndirectBuffer(
				m_draw_commands_buffer,
				m_draw_commands.data(),
				sizeof(VkDrawIndexedIndirectCommand) * draw_count,
				0
			);
		}

		void MeshManager::CreatePackedData()
		{
			m_per_draw_data.clear();

			if ((u32)m_mesh_instances_allocation.size() == 0)
			{
				return;
			}

			for (u32 i = 0; i < (u32)m_mesh_instances_allocation.size(); i++)
			{
				InstanceData data = InstanceData();

				data.material_index = m_mesh_instances_allocation[i]->mesh_index;
				m_per_draw_data.push_back(data);
			}

			u32 count = (u32)m_mesh_instances_allocation.size();

			if (m_per_draw_data_buffer)
			{
				VulkanBuffer* oldBuffer = m_per_draw_data_buffer;
				m_deletion_queue.PushFunction([backend = m_backend, oldBuffer]() {
					backend->DestroyBuffer(oldBuffer);
					});
			}

			m_per_draw_data_buffer = m_backend->CreateStorageBufferMappedPersistent(
				sizeof(InstanceData),
				count
			);

			m_backend->UpdateStorageBuffer(
				m_per_draw_data_buffer,
				m_per_draw_data.data(),
				sizeof(InstanceData) * count,
				0
			);

			DescriptorSetWriteData per_draw_data_write{};
			per_draw_data_write.type = DescriptorType_StorageBuffer;
			per_draw_data_write.binding = 0;

			VkBuffer buffer_handle = m_per_draw_data_buffer->GetHandle();
			VkDeviceSize offset = 0;
			VkDeviceSize range = sizeof(InstanceData) * count;

			per_draw_data_write.data.buffer.buffers = &buffer_handle;
			per_draw_data_write.data.buffer.offsets = &offset;
			per_draw_data_write.data.buffer.ranges = &range;

			std::vector<DescriptorSetWriteData> per_draw_data_writes = { per_draw_data_write };
			m_backend->WriteDescriptor(&m_per_draw_data_descriptor, per_draw_data_writes);
		}

		void MeshManager::UpdatePerDrawData()
		{
			if (m_per_draw_data_buffer == nullptr || (u32)m_per_draw_data.size() == 0)
			{
				return;
			}

			u32 draw_id = 0;
			u32 material_offset = 0;

			u32 index = 0;
			for (auto& allocation : m_mesh_instances_allocation)
			{
				Entity entity = SceneManager::GetInstance()->GetActiveScene()->GetEntity(allocation->entity_id);

				m_per_draw_data[index].entity_id = (u32)entity.GetHandle() + 1;
				m_per_draw_data[index].model_matrix = entity.GetComponent<TransformComponent>().world_transform;
				index++;
			}

			memcpy(m_per_draw_data_buffer->GetMappedMemory(), m_per_draw_data.data(), sizeof(InstanceData) * m_per_draw_data.size());
		}

		void MeshManager::DrawMeshes(VulkanPipeline* pipeline)
		{
			if (m_materials_descriptor == nullptr || m_textures_descriptor == nullptr || m_per_draw_data_descriptor == nullptr || m_draw_commands_buffer == nullptr)
			{
				return;
			}

			m_backend->BindDescriptorSet(pipeline, m_materials_descriptor);
			m_backend->BindDescriptorSet(pipeline, m_textures_descriptor);
			m_backend->BindDescriptorSet(pipeline, m_per_draw_data_descriptor);

			VkDeviceSize indirect_offset = 0;

			for (const auto& pool : m_pools)
			{
				m_backend->BindVertexBuffer(pool.vertex_buffer, 0);
				m_backend->BindIndexBuffer(pool.index_buffer, 0);

				m_backend->DrawIndexedIndirect(
					m_draw_commands_buffer,
					indirect_offset,
					(u32)m_mesh_instances_allocation.size(),
					sizeof(VkDrawIndexedIndirectCommand));

				indirect_offset += sizeof(VkDrawIndexedIndirectCommand) * (u32)m_mesh_instances_allocation.size();
			}
		}

		void MeshManager::CleanUp()
		{
			m_deletion_queue.Flush();
		}

		void MeshManager::CreatePool()
		{
			BufferPool pool;
			pool.vertex_buffer = m_backend->CreateVertexBufferEmpty(MAX_MEMORY_VERTICES);
			pool.index_buffer = m_backend->CreateIndexBufferEmpty(MAX_MEMORY_INDICES);
			pool.vertex_count = 0;
			pool.index_count = 0;

			m_pools.push_back(pool);
		}

		u64 MeshManager::ComputeMeshHash(const RawVertexData& v, const std::vector<u32>& indices)
		{
			u64 hash = 0;

			if (!v.positions.empty())  hash = fnv1a_hash_combine(hash, fnv1a_hash(v.positions.data(), v.positions.size()));
			if (!v.normals.empty())    hash = fnv1a_hash_combine(hash, fnv1a_hash(v.normals.data(), v.normals.size()));
			if (!v.tex_coords.empty()) hash = fnv1a_hash_combine(hash, fnv1a_hash(v.tex_coords.data(), v.tex_coords.size()));
			if (!v.colors.empty())     hash = fnv1a_hash_combine(hash, fnv1a_hash(v.colors.data(), v.colors.size()));
			if (!v.tangents.empty())   hash = fnv1a_hash_combine(hash, fnv1a_hash(v.tangents.data(), v.tangents.size()));
			if (!v.bitangents.empty()) hash = fnv1a_hash_combine(hash, fnv1a_hash(v.bitangents.data(), v.bitangents.size()));

			if (!indices.empty())      hash = fnv1a_hash_combine(hash, fnv1a_hash(indices.data(), indices.size()));

			hash = fnv1a_hash_combine(hash, (u64)v.positions.size());
			hash = fnv1a_hash_combine(hash, (u64)indices.size());

			return hash;
		}

	} // namespace graphics

} // namespace hellengine
