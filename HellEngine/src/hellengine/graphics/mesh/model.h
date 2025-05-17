#pragma once

// Internal
#include "mesh.h"

namespace hellengine
{

	using namespace math;
	namespace graphics
	{

		class Model
		{
		public:
			HE_API Model();
			HE_API ~Model();

			void AddMesh(Mesh* mesh) { m_meshes.push_back(mesh); }
			HE_API void GenerateDescriptorSets(VulkanPipeline* pipeline, u32 set);

			std::vector<Mesh*>& GetMeshes() { return m_meshes; }
			const std::vector<Mesh*>& GetMeshes() const { return m_meshes; }

			std::vector<RawVertexData>& GetVertices() { return m_vertices; }
			const std::vector<RawVertexData>& GetVertices() const { return m_vertices; }

			std::vector<u32>& GetIndices() { return m_indices; }
			const std::vector<u32>& GetIndices() const { return m_indices; }

			void SetVertexBuffer(VulkanBuffer* vertex_buffer) { m_vertex_buffer = vertex_buffer; }

			VulkanBuffer* GetVertexBuffer() { return m_vertex_buffer; }
			const VulkanBuffer* GetVertexBuffer() const { return m_vertex_buffer; }

			void SetIndexBuffer(VulkanBuffer* index_buffer) { m_index_buffer = index_buffer; }

			VulkanBuffer* GetIndexBuffer() { return m_index_buffer; }
			const VulkanBuffer* GetIndexBuffer() const { return m_index_buffer; }

			template<typename VertexT> HE_API void UploadToGPU();
			HE_API void Draw(VulkanPipeline* pipeline, u32 pipeline_index);
		
		private:
			std::vector<Mesh*> m_meshes;
			std::vector<VulkanDescriptorSet*> m_descriptor_sets;

			std::vector<RawVertexData> m_vertices;
			std::vector<u32> m_indices;

			VulkanBuffer* m_vertex_buffer;
			VulkanBuffer* m_index_buffer;

			VulkanBackend* m_backend;
		};

	} // namespace graphics

} // namespace hellengine