#pragma once

// Internal
#include <hellengine/core/core.h>
#include <hellengine/graphics/backend/vulkan_backend.h>
#include <hellengine/graphics/mesh/mesh.h>

namespace hellengine
{

	using namespace math;
	namespace graphics
	{

		constexpr u32 MAX_VERTICES = 1000000;
		constexpr u32 MAX_INDICES = MAX_VERTICES * 6;
		constexpr u64 MIN_MEMORY_ALIGNMENT = lcm_array<2>({ sizeof(VertexFormatBase), sizeof(VertexFormatTangent) });
		constexpr u64 MAX_MEMORY_VERTICES = MAX_VERTICES * MIN_MEMORY_ALIGNMENT;
		constexpr u64 MAX_MEMORY_INDICES = MAX_INDICES * MIN_MEMORY_ALIGNMENT;

		struct BufferPool
		{
			VulkanBuffer* vertex_buffer = nullptr;
			VulkanBuffer* index_buffer = nullptr;
			u32 vertex_count = 0;
			u32 index_count = 0;

			std::vector<Mesh*> meshes;
		};

		class Mesh;

		class MeshManager
		{
		public:
			void Init(VulkanBackend* backend);
			void Shutdown();

			template<typename VertexT>
			b8 CreateMesh(std::string name, RawVertexData vertices, std::vector<u32> indices, MaterialInfo* material);

			void GenerateResources(VulkanPipeline* pipeline, u32 set, TextureType types);

			void DrawMesh(std::string name, u32 instance_count = 1);
			void DrawAll(VulkanPipeline* pipeline);

			VulkanBackend* GetBackend() { return m_backend; }
			static MeshManager* GetInstance();

		private:
			void CreateNewPool();

		private:
			struct MeshAllocation
			{
				Mesh* mesh;
				u32 pool_index;
			};
			std::unordered_map<std::string, MeshAllocation> m_mesh_allocations;

			std::vector<BufferPool> m_pools;
			u32 m_current_pool_index = 0;

			VulkanDescriptorSet* m_textures_descriptor;

			VulkanStorageBuffer* m_materials_buffer;
			VulkanDescriptorSet* m_materials_descriptor;
			std::vector<MaterialGPUInfo> m_mesh_gpu_info;

			VulkanBackend* m_backend;
		};

	} // namespace graphics

} // namespace hellengine