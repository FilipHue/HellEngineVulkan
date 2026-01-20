#pragma once

// Internal
#include <hellengine/core/core.h>
#include <hellengine/core/uuid/uuid.h>

#include <hellengine/graphics/backend/vulkan_backend.h>
#include <hellengine/graphics/mesh/mesh.h>

namespace hellengine
{

	using namespace math;
	namespace graphics
	{

		constexpr u32 MAX_MATERIALS = 10000;
		constexpr u32 MAX_TEXTURES = 100000;
		constexpr u32 MAX_VERTICES = 1000000;
		constexpr u32 MAX_INDICES = MAX_VERTICES * 6;
		constexpr u64 MIN_MEMORY_ALIGNMENT = lcm_array<2>({ sizeof(VertexFormatBase), sizeof(VertexFormatTangent) });
		constexpr u64 MAX_MEMORY_VERTICES = MAX_VERTICES * MIN_MEMORY_ALIGNMENT;
		constexpr u64 MAX_MEMORY_INDICES = MAX_INDICES * MIN_MEMORY_ALIGNMENT;

		constexpr u32 SHADER_MATERIALS_BINDING = 1;
		constexpr u32 SHADER_PER_DRAW_DATA_BINDING = 2;
		constexpr u32 SHADER_TEXTURES_BINDING = 3;

		struct BufferPool
		{
			VulkanBuffer* vertex_buffer = nullptr;
			VulkanBuffer* index_buffer = nullptr;
			u32 vertex_count = 0;
			u32 index_count = 0;

			std::vector<u32> mesh_list;
		};

		struct MeshBufferAllocation
		{
			u32 pool_index = 0;

			u32 index_offset = 0;
			u32 index_count = 0;
			i32 vertex_offset = 0;
			i32 vertex_count = 0;

			u32 ref_count = 0;
			u32 mesh_index = 0;
		};

		struct MeshInstanceAllocation
		{
			u32 mesh_index = 0;
			UUID entity_id;
			MaterialInfo* material_info = nullptr;
			MeshBufferAllocation* buffer_allocation = nullptr;
		};

		ALIGN_AS(16) struct InstanceData
		{
			glm::mat4 model_matrix;
			u32 material_index;
			i32 entity_id;
		};

		struct DeletionQueue
		{
			std::vector<std::function<void()>> deletors;
			void PushFunction(std::function<void()> function)
			{
				deletors.push_back(function);
			}
			void Flush()
			{
				for (auto it = deletors.rbegin(); it != deletors.rend(); ++it)
				{
					(*it)();
				}
				deletors.clear();
			}
		};

		class MeshManager
		{
		public:
			using MeshHash = u64;

		public:
			void Init(VulkanBackend* backend);
			void Shutdown();

			void CreateDescriptors(VulkanPipeline* pipeline, u32 set);

			Mesh* CreateMesh(std::string name, RawVertexData vertices, std::vector<u32> indices, MaterialInfo* material);
			void CreateMeshInstance(UUID id, Mesh* mesh, MaterialInfo* material = nullptr);
			void RemoveMeshInstance(UUID id);

			template<typename VertexT>
			b8 UploadMeshGeometry(Mesh* mesh);

			void UploadToGpu(VulkanPipeline* pipeline, u32 set, TextureType types);
			void CreateDrawCommands();
			void CreatePackedData();

			void UpdatePerDrawData();
			void DrawMeshes(VulkanPipeline* pipeline);
			void CleanUp();

			std::vector<Mesh*>& GetAllMeshes() { return m_meshes; }
			const std::vector<Mesh*>& GetAllMeshes() const { return m_meshes; }

			void SetMeshInstanceFilter(UUID id, Mesh* mesh);

			VulkanBackend* GetBackend() { return m_backend; }
			static MeshManager* GetInstance();

		private:
			void CreatePool();
			u64 ComputeMeshHash(const RawVertexData& v, const std::vector<u32>& indices);

		private:
			std::vector<Mesh*> m_meshes;
			std::unordered_map<MeshHash, MeshBufferAllocation> m_meshes_buffer_map;
			std::vector<MeshInstanceAllocation*> m_mesh_instances_allocation;
			std::unordered_map<UUID, u32> m_mesh_instance_allocation_map;
			u32 m_last_mesh_index = 0;

			std::vector<BufferPool> m_pools;

			std::vector<MaterialGPUInfo> m_mesh_gpu_info;
			VulkanDescriptorSet* m_textures_descriptor = nullptr;
			VulkanStorageBuffer* m_materials_buffer = nullptr;
			VulkanDescriptorSet* m_materials_descriptor = nullptr;
			u32 m_last_texture_index = 0;

			std::vector<VkDrawIndexedIndirectCommand> m_draw_commands;
			VulkanBuffer* m_draw_commands_buffer = nullptr;

			std::vector<InstanceData> m_per_draw_data;
			VulkanStorageBuffer* m_per_draw_data_buffer = nullptr;
			VulkanDescriptorSet* m_per_draw_data_descriptor = nullptr;

			std::vector<VkImageView> m_descriptor_image_views;
			std::vector<VkSampler>   m_descriptor_image_samplers;

			DeletionQueue m_deletion_queue;
			VulkanBackend* m_backend = nullptr;
		};


	} // namespace graphics

} // namespace hellengine
