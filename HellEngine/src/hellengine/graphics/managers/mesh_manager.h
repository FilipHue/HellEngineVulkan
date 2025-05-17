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

		constexpr u32 MAX_VERTICES = 65536;
		constexpr u32 MAX_INDICES = MAX_VERTICES * 6;
		constexpr u64 MIN_MEMORY_ALIGNMENT = lcm_array<2>({ sizeof(VertexFormatBase), sizeof(VertexFormatTangent) });
		constexpr u64 MAX_MEMORY_VERTICES = MAX_VERTICES * MIN_MEMORY_ALIGNMENT;
		constexpr u64 MAX_MEMORY_INDICES = MAX_INDICES * MIN_MEMORY_ALIGNMENT;

		class Mesh;

		class MeshManager
		{
		public:
			void Init(VulkanBackend* backend);
			void Shutdown();

			b8 CreateMesh(std::string name, std::vector<VertexFormatBase> vertices, std::vector<u32> indices);

			void DrawMesh(std::string name, u32 instance_count = 1);

			VulkanBackend* GetBackend() { return m_backend; }
			static MeshManager* GetInstance();

		private:
			std::unordered_map<std::string, Mesh*> m_meshes;

			VulkanBuffer* m_global_vertex_buffer;
			VulkanBuffer* m_global_index_buffer;

			u32 m_global_vertex_count;
			u32 m_global_index_count;

			VulkanBackend* m_backend;
		};

	} // namespace graphics

} // namespace hellengine