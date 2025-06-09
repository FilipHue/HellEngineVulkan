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
			Model();
			~Model();

			void Init(VulkanBackend* backend);
			void Destroy();

			void AddMesh(Mesh* mesh) { m_meshes.push_back(mesh); }

			std::vector<Mesh*>& GetMeshes() { return m_meshes; }
			const std::vector<Mesh*>& GetMeshes() const { return m_meshes; }

			std::vector<VulkanTexture*>& GetTextures() { return m_textures; }
			const std::vector<VulkanTexture*>& GetTextures() const { return m_textures; }

			RawVertexData& GetVerticesRawData() { return m_vertices_raw_data; }
			const RawVertexData& GetVerticesRawData() const { return m_vertices_raw_data; }

			std::vector<u32>& GetIndices() { return m_indices; }
			const std::vector<u32>& GetIndices() const { return m_indices; }

			void SetVertexBuffer(VulkanBuffer* vertex_buffer) { m_vertex_buffer = vertex_buffer; }
			VulkanBuffer* GetVertexBuffer() { return m_vertex_buffer; }
			const VulkanBuffer* GetVertexBuffer() const { return m_vertex_buffer; }

			void SetIndexBuffer(VulkanBuffer* index_buffer) { m_index_buffer = index_buffer; }
			VulkanBuffer* GetIndexBuffer() { return m_index_buffer; }
			const VulkanBuffer* GetIndexBuffer() const { return m_index_buffer; }

			template<typename VertexT> HE_API void UploadToGPU();
			HE_API void GenerateModelResources(VulkanPipeline* pipeline, u32 set, TextureType types);
			HE_API void Draw(VulkanPipeline* pipeline, u32 pipeline_index);
		
		private:
			std::vector<Mesh*> m_meshes;
			std::vector<VulkanTexture*> m_textures;

			RawVertexData m_vertices_raw_data;
			std::vector<u32> m_indices;

			VulkanBuffer* m_vertex_buffer;
			VulkanBuffer* m_index_buffer;

			VulkanDescriptorSet* m_textures_descriptor;
			std::vector<VulkanDescriptorSet*> m_meshes_descriptor;
			VulkanUniformBuffer* m_meshes_buffer;
			std::vector<MaterialGPUInfo> m_mesh_gpu_info;

			VulkanBackend* m_backend;
		};

	} // namespace graphics

} // namespace hellengine