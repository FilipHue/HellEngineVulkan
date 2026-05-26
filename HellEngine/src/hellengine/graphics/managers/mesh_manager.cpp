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

		// ================================================================
		// Init / Shutdown
		// ================================================================

		void MeshManager::Init(VulkanBackend* backend)
		{
			m_backend = backend;

			m_pools.clear();
			m_pools.push_back({
				m_backend->CreateVertexBufferEmpty(MAX_MEMORY_VERTICES),
				m_backend->CreateIndexBufferEmpty(MAX_MEMORY_INDICES),
				0, 0,
				m_backend->CreateStorageBufferMappedPersistent(sizeof(VertexFormatTangent), MAX_VERTICES),
				m_backend->CreateStorageBufferMappedPersistent(sizeof(u32),                 MAX_INDICES),
				m_backend->CreateStorageBufferMappedPersistent(sizeof(MeshletData),         MAX_MESHLETS),
				0, 0, 0
				});

			m_materials_buffer              = m_backend->CreateStorageBufferMappedPersistent(sizeof(MaterialGPUInfo), MAX_MATERIALS);
			m_materials_descriptor          = nullptr;
			m_textures_descriptor           = nullptr;
			m_last_texture_index            = 0;

			m_draw_commands_buffer          = nullptr;
			m_mesh_task_commands_buffer     = nullptr;
			m_mesh_shader_draw_args_buffer  = nullptr;
			m_per_draw_data_buffer          = nullptr;
			m_per_draw_data_descriptor      = nullptr;
			m_per_draw_data_descriptor_mesh = nullptr;

			m_mesh_instances_allocation.clear();
			m_mesh_instance_allocation_map.clear();

			m_mesh_gpu_info.clear();
			m_draw_commands.clear();
			m_mesh_task_commands.clear();
			m_mesh_shader_draw_args.clear();
			m_per_draw_data.clear();

			m_descriptor_image_views.clear();
			m_descriptor_image_samplers.clear();

			m_geometry_mode   = GeometryMode_Classic;
			m_last_mesh_index = 0;
		}

		void MeshManager::Shutdown()
		{
			CleanUp();

			for (auto& pool : m_pools)
			{
				m_backend->DestroyBuffer(pool.vertex_buffer);
				m_backend->DestroyBuffer(pool.index_buffer);

				if (pool.ms_vertex_buffer)  m_backend->DestroyBuffer(pool.ms_vertex_buffer);
				if (pool.ms_index_buffer)   m_backend->DestroyBuffer(pool.ms_index_buffer);
				if (pool.ms_meshlet_buffer) m_backend->DestroyBuffer(pool.ms_meshlet_buffer);

				pool.mesh_list.clear();
			}
			m_pools.clear();

			m_mesh_instances_allocation.clear();
			m_mesh_instance_allocation_map.clear();

			if (m_materials_buffer)            { m_backend->DestroyBuffer(m_materials_buffer);             m_materials_buffer = nullptr; }
			if (m_draw_commands_buffer)         { m_backend->DestroyBuffer(m_draw_commands_buffer);         m_draw_commands_buffer = nullptr; }
			if (m_mesh_task_commands_buffer)    { m_backend->DestroyBuffer(m_mesh_task_commands_buffer);    m_mesh_task_commands_buffer = nullptr; }
			if (m_mesh_shader_draw_args_buffer) { m_backend->DestroyBuffer(m_mesh_shader_draw_args_buffer); m_mesh_shader_draw_args_buffer = nullptr; }
			if (m_per_draw_data_buffer)         { m_backend->DestroyBuffer(m_per_draw_data_buffer);         m_per_draw_data_buffer = nullptr; }
		}

		// ================================================================
		// Descriptor creation
		// ================================================================

		void MeshManager::CreateDescriptors(VulkanPipeline* classic_pipeline, VulkanPipeline* mesh_shader_pipeline)
		{
			// Set 1 — materials SSBO
			{
				m_materials_descriptor = m_backend->CreateDescriptorSet(classic_pipeline, SHADER_MATERIALS_BINDING);

				VkBuffer     buffer_handle = m_materials_buffer->GetHandle();
				VkDeviceSize offset        = 0;
				VkDeviceSize range         = MAX_MATERIALS * sizeof(MaterialGPUInfo);

				DescriptorSetWriteData buffer_data{};
				buffer_data.type                = DescriptorType_StorageBuffer;
				buffer_data.binding             = 0;
				buffer_data.data.buffer.buffers = &buffer_handle;
				buffer_data.data.buffer.offsets = &offset;
				buffer_data.data.buffer.ranges  = &range;

				std::vector<DescriptorSetWriteData> writes = { buffer_data };
				m_backend->WriteDescriptor(&m_materials_descriptor, writes);
			}

			// Set 2 — two descriptors, one per pipeline layout.
			// Classic: binding 0 only. Mesh: bindings 0-4.
			// Actual writes happen in CreatePackedData (binding 0) and
			// UpdateMeshShaderDescriptors (bindings 1-4, called after geometry upload).
			{
				m_per_draw_data_descriptor      = m_backend->CreateDescriptorSet(classic_pipeline,     SHADER_PER_DRAW_DATA_BINDING);
				m_per_draw_data_descriptor_mesh = m_backend->CreateDescriptorSet(mesh_shader_pipeline, SHADER_PER_DRAW_DATA_BINDING);
			}

			// Set 3 — variable-count texture array. Must be last set.
			{
				m_textures_descriptor = m_backend->CreateDescriptorSetVariable(classic_pipeline, SHADER_TEXTURES_BINDING, { MAX_TEXTURES });
			}
		}

		// ================================================================
		// Mesh / instance lifecycle
		// ================================================================

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
				mesh_index = static_cast<u32>(std::distance(m_meshes.begin(), it));

			allocation->material_info     = material != nullptr ? material : mesh->GetMaterialInfo();
			allocation->mesh_index        = mesh_index;
			allocation->entity_id         = id;
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
			u32                     allocation_index = it->second;
			MeshInstanceAllocation* allocation       = m_mesh_instances_allocation[allocation_index];
			Mesh*                   mesh             = m_meshes[allocation->mesh_index];
			MeshHash                hash             = ComputeMeshHash(mesh->GetRawData(), mesh->GetIndices());
			auto git = m_meshes_buffer_map.find(hash);
			if (git != m_meshes_buffer_map.end())
			{
				if (git->second.ref_count > 0)
					git->second.ref_count--;
			}
			delete allocation;
			m_mesh_instances_allocation.erase(m_mesh_instances_allocation.begin() + allocation_index);
			m_mesh_instance_allocation_map.erase(it);

			for (u32 i = allocation_index; i < m_mesh_instances_allocation.size(); i++)
				m_mesh_instance_allocation_map[m_mesh_instances_allocation[i]->entity_id] = i;

			CreateDrawCommands();
			CreatePackedData();
		}

		// ================================================================
		// UploadMeshGeometry
		// ================================================================

		template b8 MeshManager::UploadMeshGeometry<VertexFormatSimple>(Mesh* mesh);
		template b8 MeshManager::UploadMeshGeometry<VertexFormatTangent>(Mesh* mesh);

		template<typename VertexT>
		b8 MeshManager::UploadMeshGeometry(Mesh* mesh)
		{
			RawVertexData&    vertices     = mesh->GetRawData();
			std::vector<u32>& indices      = mesh->GetIndices();
			const u32         vertex_count = (u32)vertices.positions.size();
			const u32         index_count  = (u32)indices.size();

			if (vertex_count > MAX_VERTICES || index_count > MAX_INDICES)
			{
				HE_GRAPHICS_ERROR(
					"Mesh has more vertices or indices than the maximum allowed per pool!\n"
					"\tMesh has {0} vertices and {1} indices\n"
					"\tMaximum allowed is {2} vertices and {3} indices",
					vertex_count, index_count, MAX_VERTICES, MAX_INDICES);
				return false;
			}

			MeshHash hash = ComputeMeshHash(vertices, indices);
			if (m_meshes_buffer_map.find(hash) != m_meshes_buffer_map.end())
			{
				HE_GRAPHICS_WARN("Mesh geometry already uploaded, skipping upload.");
				return true;
			}

			BufferPool* current_pool       = nullptr;
			u32         current_pool_index = 0;
			for (auto& pool : m_pools)
			{
				if (pool.vertex_count + vertex_count <= MAX_VERTICES &&
					pool.index_count  + index_count  <= MAX_INDICES)
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
				current_pool       = &m_pools.back();
				current_pool_index = (u32)(m_pools.size() - 1);
			}

			m_meshes.push_back(mesh);
			current_pool->mesh_list.push_back((u32)m_meshes.size() - 1);

			std::vector<VertexT> final_vertices;
			final_vertices.reserve(vertices.positions.size());
			for (size_t i = 0; i < vertices.positions.size(); ++i)
			{
				VertexT vtx{};
				if constexpr (requires { vtx.position;  }) vtx.position  = vertices.positions[i];
				if constexpr (requires { vtx.color;     }) vtx.color     = vertices.colors.size()     ? vertices.colors[i]     : glm::vec4(1.0f);
				if constexpr (requires { vtx.tex_coord; }) vtx.tex_coord = vertices.tex_coords.size() ? vertices.tex_coords[i] : glm::vec2(0.0f);
				if constexpr (requires { vtx.normal;    }) vtx.normal    = vertices.normals.size()    ? vertices.normals[i]    : glm::vec3(0.0f);
				if constexpr (requires { vtx.tangent;   }) vtx.tangent   = vertices.tangents.size()   ? vertices.tangents[i]   : glm::vec3(0.0f);
				if constexpr (requires { vtx.bitangent; }) vtx.bitangent = vertices.bitangents.size() ? vertices.bitangents[i] : glm::vec3(0.0f);
				final_vertices.push_back(vtx);
			}

			m_backend->UpdateVertexBuffer(
				current_pool->vertex_buffer,
				current_pool->vertex_count * sizeof(VertexT),
				final_vertices.data(),
				vertex_count * sizeof(VertexT));

			m_backend->UpdateIndexBuffer(
				current_pool->index_buffer,
				current_pool->index_count * sizeof(u32),
				indices.data(),
				index_count * sizeof(u32));

			MeshBufferAllocation alloc{};
			alloc.index_count   = index_count;
			alloc.index_offset  = current_pool->index_count;
			alloc.vertex_count  = vertex_count;
			alloc.vertex_offset = current_pool->vertex_count;
			alloc.pool_index    = current_pool_index;
			alloc.ref_count     = 0;
			alloc.mesh_index    = (u32)m_meshes.size() - 1;

			BuildMeshletsForMesh(mesh, alloc, *current_pool);

			m_meshes_buffer_map[hash] = alloc;

			current_pool->vertex_count += vertex_count;
			current_pool->index_count  += index_count;

			return true;
		}

		// ================================================================
		// BuildMeshletsForMesh
		// Vertices written in meshlet-local order so the mesh shader
		// addresses them as vertices[meshlet.vertex_offset + local_v].
		// Uses UpdateStorageBuffer (host-visible mapped memory).
		// ================================================================

		void MeshManager::BuildMeshletsForMesh(Mesh* mesh, MeshBufferAllocation& alloc, BufferPool& pool)
		{
			RawVertexData&    raw            = mesh->GetRawData();
			std::vector<u32>& indices        = mesh->GetIndices();
			const u32         triangle_count = (u32)indices.size() / 3;

			alloc.ms_meshlet_offset = pool.ms_meshlet_count;
			alloc.ms_vertex_offset  = pool.ms_vertex_count;
			alloc.ms_index_offset   = pool.ms_index_count;

			std::vector<MeshletData>         meshlets;
			std::vector<u32>                 all_prim_indices;
			std::vector<VertexFormatTangent> all_verts_ordered;

			u32 tri_cursor       = 0;
			u32 running_vert_off = 0;

			while (tri_cursor < triangle_count)
			{
				MeshletData m{};
				m.vertex_offset  = alloc.ms_vertex_offset + running_vert_off;
				m.index_offset   = alloc.ms_index_offset  + (u32)all_prim_indices.size();
				m.vertex_count   = 0;
				m.triangle_count = 0;

				std::unordered_map<u32, u32>     local_remap;
				std::vector<VertexFormatTangent> local_verts;
				glm::vec3 center(0.0f);

				while (tri_cursor < triangle_count &&
					   m.triangle_count < MESHLET_MAX_TRIANGLES)
				{
					u32 i0 = indices[tri_cursor * 3 + 0];
					u32 i1 = indices[tri_cursor * 3 + 1];
					u32 i2 = indices[tri_cursor * 3 + 2];

					u32 new_verts = 0;
					if (local_remap.find(i0) == local_remap.end()) new_verts++;
					if (local_remap.find(i1) == local_remap.end()) new_verts++;
					if (local_remap.find(i2) == local_remap.end()) new_verts++;

					if (m.vertex_count + new_verts > MESHLET_MAX_VERTICES)
						break;

					auto add_vert = [&](u32 gidx) -> u32
					{
						auto it = local_remap.find(gidx);
						if (it != local_remap.end()) return it->second;

						u32 local_idx = m.vertex_count++;
						local_remap[gidx] = local_idx;

						VertexFormatTangent vtx{};
						vtx.position  = raw.positions[gidx];
						vtx.color     = raw.colors.size()     ? raw.colors[gidx]     : glm::vec4(1.0f);
						vtx.tex_coord = raw.tex_coords.size() ? raw.tex_coords[gidx] : glm::vec2(0.0f);
						vtx.normal    = raw.normals.size()    ? raw.normals[gidx]    : glm::vec3(0.0f);
						vtx.tangent   = raw.tangents.size()   ? raw.tangents[gidx]   : glm::vec3(0.0f);
						vtx.bitangent = raw.bitangents.size() ? raw.bitangents[gidx] : glm::vec3(0.0f);
						local_verts.push_back(vtx);

						center += raw.positions[gidx];
						return local_idx;
					};

					u32 l0 = add_vert(i0);
					u32 l1 = add_vert(i1);
					u32 l2 = add_vert(i2);

					all_prim_indices.push_back(l0);
					all_prim_indices.push_back(l1);
					all_prim_indices.push_back(l2);

					m.triangle_count++;
					tri_cursor++;
				}

				if (m.triangle_count == 0)
					break;

				center /= static_cast<f32>(m.vertex_count);
				f32 max_dist2 = 0.0f;
				for (auto& vtx : local_verts)
				{
					glm::vec3 d = vtx.position - center;
					max_dist2 = glm::max(max_dist2, glm::dot(d, d));
				}
				m.bounding_sphere_center = center;
				m.bounding_sphere_radius = glm::sqrt(max_dist2);

				for (auto& vtx : local_verts)
					all_verts_ordered.push_back(vtx);

				running_vert_off += m.vertex_count;
				meshlets.push_back(m);
			}

			alloc.ms_meshlet_count = (u32)meshlets.size();

			if (!all_verts_ordered.empty())
			{
				m_backend->UpdateStorageBuffer(
					pool.ms_vertex_buffer,
					all_verts_ordered.data(),
					running_vert_off * sizeof(VertexFormatTangent),
					pool.ms_vertex_count * sizeof(VertexFormatTangent));
				pool.ms_vertex_count += running_vert_off;
			}

			if (!all_prim_indices.empty())
			{
				m_backend->UpdateStorageBuffer(
					pool.ms_index_buffer,
					all_prim_indices.data(),
					(u32)(all_prim_indices.size() * sizeof(u32)),
					pool.ms_index_count * sizeof(u32));
				pool.ms_index_count += (u32)all_prim_indices.size();
			}

			if (!meshlets.empty())
			{
				m_backend->UpdateStorageBuffer(
					pool.ms_meshlet_buffer,
					meshlets.data(),
					(u32)(meshlets.size() * sizeof(MeshletData)),
					pool.ms_meshlet_count * sizeof(MeshletData));
				pool.ms_meshlet_count += (u32)meshlets.size();
			}
		}

		// ================================================================
		// SetMeshInstanceFilter
		// ================================================================

		void MeshManager::SetMeshInstanceFilter(UUID id, Mesh* mesh)
		{
			auto it = m_mesh_instance_allocation_map.find(id);
			if (it == m_mesh_instance_allocation_map.end())
			{
				HE_GRAPHICS_DEBUG("Mesh instance with given UUID not found, creating new instance.");
				CreateMeshInstance(id, mesh);
				it = m_mesh_instance_allocation_map.find(id);
			}
			u32                     allocation_index = it->second;
			MeshInstanceAllocation* allocation       = m_mesh_instances_allocation[allocation_index];

			if (!mesh)
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
			allocation->mesh_index        = git->second.mesh_index;
			allocation->buffer_allocation = &git->second;
			CreateDrawCommands();
			CreatePackedData();
		}

		// ================================================================
		// UploadToGpu
		// ================================================================

		void MeshManager::UploadToGpu(TextureType types)
		{
			if (!m_materials_descriptor || !m_textures_descriptor)
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
					info.diffuse_index           = material_info->Get(TextureType_Diffuse           & types);
					info.specular_index          = material_info->Get(TextureType_Specular          & types);
					info.ambient_index           = material_info->Get(TextureType_Ambient           & types);
					info.emissive_index          = material_info->Get(TextureType_Emissive          & types);
					info.height_index            = material_info->Get(TextureType_Height            & types);
					info.normal_index            = material_info->Get(TextureType_Normals           & types);
					info.shininess_index         = material_info->Get(TextureType_Shininess         & types);
					info.opacity_index           = material_info->Get(TextureType_Opacity           & types);
					info.displacement_index      = material_info->Get(TextureType_Displacement      & types);
					info.lightmap_index          = material_info->Get(TextureType_Lightmap          & types);
					info.reflection_index        = material_info->Get(TextureType_Reflection        & types);
					info.base_color_index        = material_info->Get(TextureType_BaseColor         & types);
					info.normal_camera_index     = material_info->Get(TextureType_NormalCamera      & types);
					info.emission_color_index    = material_info->Get(TextureType_EmissionColor     & types);
					info.metalness_index         = material_info->Get(TextureType_Metalness         & types);
					info.diffuse_roughness_index = material_info->Get(TextureType_DiffuseRoughness  & types);
					info.ambient_occlusion_index = material_info->Get(TextureType_AmbientOcclusion  & types);
					info.sheen_index             = material_info->Get(TextureType_Sheen             & types);
					info.clearcoat_index         = material_info->Get(TextureType_Clearcoat         & types);
					info.transmission_index      = material_info->Get(TextureType_Transmission      & types);

					m_mesh_gpu_info.push_back(info);
				}

				void* data = &m_mesh_gpu_info[m_last_mesh_index];
				m_backend->UpdateStorageBuffer(
					m_materials_buffer,
					data,
					sizeof(MaterialGPUInfo) * ((u32)m_mesh_gpu_info.size() - m_last_mesh_index),
					sizeof(MaterialGPUInfo) * m_last_mesh_index);
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
					}

					if (!m_descriptor_image_views.empty())
					{
						DescriptorSetWriteData image_data{};
						image_data.type    = DescriptorType_CombinedImageSampler;
						image_data.binding = 2;
						image_data.data.image.image_views = m_descriptor_image_views.data();
						image_data.data.image.samplers    = m_descriptor_image_samplers.data();

						std::vector<DescriptorSetWriteData> writes = { image_data };
						m_backend->WriteDescriptorVariable(
							&m_textures_descriptor,
							writes,
							(u32)m_descriptor_image_views.size(),
							0);
					}

					m_last_texture_index = texCount;
				}
			}

			CreateDrawCommands();
			CreatePackedData();
			UpdatePerDrawData();

			// After geometry and draw commands are built, refresh mesh descriptor
			if (m_geometry_mode == GeometryMode_MeshShader)
				UpdateMeshShaderDescriptors();
		}

		// ================================================================
		// SetGeometryMode
		// ================================================================

		void MeshManager::SetGeometryMode(GeometryMode mode)
		{
			if (m_geometry_mode == mode)
				return;

			m_geometry_mode = mode;
			CreateDrawCommands();

			if (m_geometry_mode == GeometryMode_MeshShader)
				UpdateMeshShaderDescriptors();
		}

		// ================================================================
		// UpdateMeshShaderDescriptors
		// Writes all 5 bindings of set 2 mesh descriptor in one call so
		// WriteDescriptor's internal Clear() doesn't wipe any binding.
		// ================================================================

		void MeshManager::UpdateMeshShaderDescriptors()
		{
			if (!m_per_draw_data_descriptor_mesh || m_pools.empty())
				return;

			BufferPool& pool = m_pools[0];

			if (pool.ms_vertex_count == 0 || pool.ms_index_count == 0 || pool.ms_meshlet_count == 0)
				return;

			if (!m_mesh_shader_draw_args_buffer || !m_per_draw_data_buffer)
				return;

			const u32 total_commands = (u32)m_mesh_task_commands.size();
			if (total_commands == 0)
				return;

			// Binding 0 — InstanceData
			VkBuffer     pd_buf   = m_per_draw_data_buffer->GetHandle();
			VkDeviceSize pd_off   = 0;
			VkDeviceSize pd_range = sizeof(InstanceData) * (u32)m_per_draw_data.size();

			DescriptorSetWriteData w0{};
			w0.type    = DescriptorType_StorageBuffer;
			w0.binding = 0;
			w0.data.buffer.buffers = &pd_buf;
			w0.data.buffer.offsets = &pd_off;
			w0.data.buffer.ranges  = &pd_range;

			// Bindings 1-4 — meshlet geometry SSBOs
			VkBuffer     vb    = pool.ms_vertex_buffer->GetHandle();
			VkBuffer     ib    = pool.ms_index_buffer->GetHandle();
			VkBuffer     mb    = pool.ms_meshlet_buffer->GetHandle();
			VkBuffer     ab    = m_mesh_shader_draw_args_buffer->GetHandle();
			VkDeviceSize off   = 0;
			VkDeviceSize vb_sz = pool.ms_vertex_count  * sizeof(VertexFormatTangent);
			VkDeviceSize ib_sz = pool.ms_index_count   * sizeof(u32);
			VkDeviceSize mb_sz = pool.ms_meshlet_count * sizeof(MeshletData);
			VkDeviceSize ab_sz = total_commands        * sizeof(MeshShaderDrawArgs);

			DescriptorSetWriteData w1{};
			w1.type = DescriptorType_StorageBuffer; w1.binding = SHADER_MESHLET_VERTICES_BINDING;
			w1.data.buffer.buffers = &vb; w1.data.buffer.offsets = &off; w1.data.buffer.ranges = &vb_sz;

			DescriptorSetWriteData w2{};
			w2.type = DescriptorType_StorageBuffer; w2.binding = SHADER_MESHLET_INDICES_BINDING;
			w2.data.buffer.buffers = &ib; w2.data.buffer.offsets = &off; w2.data.buffer.ranges = &ib_sz;

			DescriptorSetWriteData w3{};
			w3.type = DescriptorType_StorageBuffer; w3.binding = SHADER_MESHLET_DESC_BINDING;
			w3.data.buffer.buffers = &mb; w3.data.buffer.offsets = &off; w3.data.buffer.ranges = &mb_sz;

			DescriptorSetWriteData w4{};
			w4.type = DescriptorType_StorageBuffer; w4.binding = SHADER_MESHLET_DRAWINFO_BINDING;
			w4.data.buffer.buffers = &ab; w4.data.buffer.offsets = &off; w4.data.buffer.ranges = &ab_sz;

			std::vector<DescriptorSetWriteData> writes = { w0, w1, w2, w3, w4 };
			m_backend->WriteDescriptor(&m_per_draw_data_descriptor_mesh, writes);
		}

		// ================================================================
		// CreateDrawCommands
		// ================================================================

		void MeshManager::CreateDrawCommands()
		{
			if (m_geometry_mode == GeometryMode_Classic)
				CreateClassicDrawCommands();
			else
				CreateMeshShaderDrawCommands();
		}

		void MeshManager::CreateClassicDrawCommands()
		{
			m_draw_commands.clear();

			std::sort(m_mesh_instances_allocation.begin(), m_mesh_instances_allocation.end(),
				[](MeshInstanceAllocation* a, MeshInstanceAllocation* b)
				{ return a->buffer_allocation->pool_index < b->buffer_allocation->pool_index; });

			u32 current_pool     = 0;
			u32 current_instance = 0;
			u32 draw_id          = 0;
			while (current_pool < (u32)m_pools.size())
			{
				m_pools[current_pool].number_of_instances = 0;
				while (current_instance < (u32)m_mesh_instances_allocation.size())
				{
					MeshInstanceAllocation* instance = m_mesh_instances_allocation[current_instance];
					if (current_pool != instance->buffer_allocation->pool_index)
						break;

					m_pools[current_pool].number_of_instances++;

					VkDrawIndexedIndirectCommand command{};
					command.indexCount    = instance->buffer_allocation->index_count;
					command.instanceCount = 1;
					command.firstIndex    = instance->buffer_allocation->index_offset;
					command.vertexOffset  = instance->buffer_allocation->vertex_offset;
					command.firstInstance = draw_id;

					m_draw_commands.push_back(command);
					current_instance++;
					draw_id++;
				}
				current_pool++;
			}

			if (m_draw_commands.empty())
			{
				HE_GRAPHICS_WARN("No classic draw commands to create!");
				return;
			}

			const u32 draw_count = (u32)m_draw_commands.size();

			if (m_draw_commands_buffer)
			{
				VulkanBuffer* old = m_draw_commands_buffer;
				m_deletion_queue.PushFunction([backend = m_backend, old]() { backend->DestroyBuffer(old); });
			}

			m_draw_commands_buffer = m_backend->CreateDrawIndirectBuffer(sizeof(VkDrawIndexedIndirectCommand), draw_count);
			m_backend->UpdateDrawIndirectBuffer(
				m_draw_commands_buffer,
				m_draw_commands.data(),
				sizeof(VkDrawIndexedIndirectCommand) * draw_count,
				0);
		}

		// ================================================================
		// CreateMeshShaderDrawCommands
		// One command per instance, group_count_x = meshlet count.
		// Shader: meshlets[draw_args[gl_DrawID].meshlet_index + gl_WorkGroupID.x]
		// ================================================================

		void MeshManager::CreateMeshShaderDrawCommands()
		{
			m_mesh_task_commands.clear();
			m_mesh_shader_draw_args.clear();

			if (m_mesh_instances_allocation.empty())
				return;

			std::sort(m_mesh_instances_allocation.begin(), m_mesh_instances_allocation.end(),
				[](MeshInstanceAllocation* a, MeshInstanceAllocation* b)
				{ return a->buffer_allocation->pool_index < b->buffer_allocation->pool_index; });

			u32 current_pool     = 0;
			u32 current_instance = 0;

			while (current_pool < (u32)m_pools.size())
			{
				m_pools[current_pool].number_of_instances = 0;
				while (current_instance < (u32)m_mesh_instances_allocation.size())
				{
					MeshInstanceAllocation* inst = m_mesh_instances_allocation[current_instance];
					if (current_pool != inst->buffer_allocation->pool_index)
						break;

					const u32 meshlet_count = inst->buffer_allocation->ms_meshlet_count;

					if (meshlet_count > 0)
					{
						m_pools[current_pool].number_of_instances++;

						MeshTaskIndirectCommand cmd{};
						cmd.group_count_x = meshlet_count;
						cmd.group_count_y = 1;
						cmd.group_count_z = 1;
						m_mesh_task_commands.push_back(cmd);

						MeshShaderDrawArgs args{};
						args.meshlet_index  = inst->buffer_allocation->ms_meshlet_offset;
						args.instance_index = current_instance;
						m_mesh_shader_draw_args.push_back(args);
					}

					current_instance++;
				}
				current_pool++;
			}

			if (m_mesh_task_commands.empty())
			{
				HE_GRAPHICS_WARN("No mesh shader draw commands to create!");
				return;
			}

			const u32 total_commands = (u32)m_mesh_task_commands.size();

			if (m_mesh_task_commands_buffer)
			{
				VulkanBuffer* old = m_mesh_task_commands_buffer;
				m_deletion_queue.PushFunction([backend = m_backend, old]() { backend->DestroyBuffer(old); });
			}

			m_mesh_task_commands_buffer = m_backend->CreateDrawIndirectBuffer(
				sizeof(MeshTaskIndirectCommand), total_commands);
			m_backend->UpdateDrawIndirectBuffer(
				m_mesh_task_commands_buffer,
				m_mesh_task_commands.data(),
				sizeof(MeshTaskIndirectCommand) * total_commands,
				0);

			if (m_mesh_shader_draw_args_buffer)
			{
				VulkanStorageBuffer* old = m_mesh_shader_draw_args_buffer;
				m_deletion_queue.PushFunction([backend = m_backend, old]() { backend->DestroyBuffer(old); });
			}

			m_mesh_shader_draw_args_buffer = m_backend->CreateStorageBufferMappedPersistent(
				sizeof(MeshShaderDrawArgs), total_commands);
			m_backend->UpdateStorageBuffer(
				m_mesh_shader_draw_args_buffer,
				m_mesh_shader_draw_args.data(),
				sizeof(MeshShaderDrawArgs) * total_commands,
				0);
		}

		// ================================================================
		// CreatePackedData
		// ================================================================

		void MeshManager::CreatePackedData()
		{
			m_per_draw_data.clear();

			if (m_mesh_instances_allocation.empty())
				return;

			for (u32 i = 0; i < (u32)m_mesh_instances_allocation.size(); i++)
			{
				InstanceData data{};
				data.material_index = m_mesh_instances_allocation[i]->mesh_index;
				m_per_draw_data.push_back(data);
			}

			const u32 count = (u32)m_mesh_instances_allocation.size();

			if (m_per_draw_data_buffer)
			{
				VulkanStorageBuffer* old = m_per_draw_data_buffer;
				m_deletion_queue.PushFunction([backend = m_backend, old]() { backend->DestroyBuffer(old); });
			}

			m_per_draw_data_buffer = m_backend->CreateStorageBufferMappedPersistent(sizeof(InstanceData), count);
			m_backend->UpdateStorageBuffer(
				m_per_draw_data_buffer,
				m_per_draw_data.data(),
				sizeof(InstanceData) * count,
				0);

			VkBuffer     buf   = m_per_draw_data_buffer->GetHandle();
			VkDeviceSize off   = 0;
			VkDeviceSize range = sizeof(InstanceData) * count;

			DescriptorSetWriteData write{};
			write.type    = DescriptorType_StorageBuffer;
			write.binding = 0;
			write.data.buffer.buffers = &buf;
			write.data.buffer.offsets = &off;
			write.data.buffer.ranges  = &range;

			std::vector<DescriptorSetWriteData> writes = { write };

			// Classic — binding 0 only
			if (m_per_draw_data_descriptor)
				m_backend->WriteDescriptor(&m_per_draw_data_descriptor, writes);

			// Mesh descriptor binding 0 is written by UpdateMeshShaderDescriptors
			// together with bindings 1-4 to avoid Clear() wiping geometry bindings
		}

		// ================================================================
		// UpdatePerDrawData
		// ================================================================

		void MeshManager::UpdatePerDrawData()
		{
			if (!m_per_draw_data_buffer || m_per_draw_data.empty())
				return;

			u32 index = 0;
			for (auto& allocation : m_mesh_instances_allocation)
			{
				Entity entity = SceneManager::GetInstance()->GetActiveScene()->GetEntity(allocation->entity_id);
				m_per_draw_data[index].entity_id    = (u32)entity.GetHandle() + 1;
				m_per_draw_data[index].model_matrix = entity.GetComponent<TransformComponent>().world_transform;
				index++;
			}

			memcpy(m_per_draw_data_buffer->GetMappedMemory(),
				   m_per_draw_data.data(),
				   sizeof(InstanceData) * m_per_draw_data.size());
		}

		// ================================================================
		// DrawMeshes
		// ================================================================

		void MeshManager::DrawMeshes(VulkanPipeline* pipeline)
		{
			if (m_geometry_mode == GeometryMode_Classic)
				DrawMeshesClassic(pipeline);
			else
				DrawMeshesMeshShader(pipeline);
		}

		void MeshManager::DrawMeshesClassic(VulkanPipeline* pipeline)
		{
			if (!m_materials_descriptor || !m_textures_descriptor ||
				!m_per_draw_data_descriptor || !m_draw_commands_buffer)
				return;

			m_backend->BindDescriptorSet(pipeline, m_materials_descriptor);
			m_backend->BindDescriptorSet(pipeline, m_per_draw_data_descriptor);
			m_backend->BindDescriptorSet(pipeline, m_textures_descriptor);

			VkDeviceSize indirect_offset = 0;
			for (const auto& pool : m_pools)
			{
				m_backend->BindVertexBuffer(pool.vertex_buffer, 0);
				m_backend->BindIndexBuffer(pool.index_buffer, 0);

				m_backend->DrawIndexedIndirect(
					m_draw_commands_buffer,
					static_cast<u32>(indirect_offset),
					pool.number_of_instances,
					sizeof(VkDrawIndexedIndirectCommand));

				indirect_offset += sizeof(VkDrawIndexedIndirectCommand) * pool.number_of_instances;
			}
		}

		void MeshManager::DrawMeshesMeshShader(VulkanPipeline* pipeline)
		{
			if (!m_materials_descriptor || !m_textures_descriptor ||
				!m_per_draw_data_descriptor_mesh || !m_mesh_task_commands_buffer)
				return;

			m_backend->BindDescriptorSet(pipeline, m_materials_descriptor);
			m_backend->BindDescriptorSet(pipeline, m_per_draw_data_descriptor_mesh);
			m_backend->BindDescriptorSet(pipeline, m_textures_descriptor);

			VkDeviceSize indirect_offset = 0;
			for (const auto& pool : m_pools)
			{
				if (pool.number_of_instances == 0)
					continue;

				m_backend->DrawMeshTasksIndirect(
					m_mesh_task_commands_buffer,
					static_cast<u32>(indirect_offset),
					pool.number_of_instances,
					sizeof(MeshTaskIndirectCommand));

				indirect_offset += sizeof(MeshTaskIndirectCommand) * pool.number_of_instances;
			}
		}

		// ================================================================
		// Helpers
		// ================================================================

		void MeshManager::CleanUp()
		{
			m_deletion_queue.Flush();
		}

		void MeshManager::CreatePool()
		{
			BufferPool pool{};
			pool.vertex_buffer = m_backend->CreateVertexBufferEmpty(MAX_MEMORY_VERTICES);
			pool.index_buffer  = m_backend->CreateIndexBufferEmpty(MAX_MEMORY_INDICES);
			pool.vertex_count  = 0;
			pool.index_count   = 0;
			pool.number_of_instances = 0;

			pool.ms_vertex_buffer  = m_backend->CreateStorageBufferMappedPersistent(sizeof(VertexFormatTangent), MAX_VERTICES);
			pool.ms_index_buffer   = m_backend->CreateStorageBufferMappedPersistent(sizeof(u32),                 MAX_INDICES);
			pool.ms_meshlet_buffer = m_backend->CreateStorageBufferMappedPersistent(sizeof(MeshletData),         MAX_MESHLETS);
			pool.ms_vertex_count   = 0;
			pool.ms_index_count    = 0;
			pool.ms_meshlet_count  = 0;

			m_pools.push_back(pool);
		}

		u64 MeshManager::ComputeMeshHash(const RawVertexData& v, const std::vector<u32>& indices)
		{
			u64 hash = 0;
			if (!v.positions.empty())  hash = fnv1a_hash_combine(hash, fnv1a_hash(v.positions.data(),  v.positions.size()));
			if (!v.normals.empty())    hash = fnv1a_hash_combine(hash, fnv1a_hash(v.normals.data(),    v.normals.size()));
			if (!v.tex_coords.empty()) hash = fnv1a_hash_combine(hash, fnv1a_hash(v.tex_coords.data(), v.tex_coords.size()));
			if (!v.colors.empty())     hash = fnv1a_hash_combine(hash, fnv1a_hash(v.colors.data(),     v.colors.size()));
			if (!v.tangents.empty())   hash = fnv1a_hash_combine(hash, fnv1a_hash(v.tangents.data(),   v.tangents.size()));
			if (!v.bitangents.empty()) hash = fnv1a_hash_combine(hash, fnv1a_hash(v.bitangents.data(), v.bitangents.size()));
			if (!indices.empty())      hash = fnv1a_hash_combine(hash, fnv1a_hash(indices.data(),      indices.size()));
			hash = fnv1a_hash_combine(hash, (u64)v.positions.size());
			hash = fnv1a_hash_combine(hash, (u64)indices.size());
			return hash;
		}

		Mesh* MeshManager::GetMeshByName(const std::string& mesh_name) const
		{
			for (Mesh* mesh : m_meshes)
			{
				if (mesh && mesh->GetName() == mesh_name)
					return mesh;
			}
			return nullptr;
		}

	} // namespace graphics

} // namespace hellengine
