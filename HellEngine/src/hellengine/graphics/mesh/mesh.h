#pragma once

// Internal
#include "material.h"
#include <hellengine/core/core.h>
#include <hellengine/graphics/backend/vulkan_backend.h>

namespace hellengine
{

	namespace graphics
	{

		struct BufferAllocation
		{
			u32 vertex_start = 0;
			u32 first_index = 0;
			u32 index_count = 0;
			u32 vertex_count = 0;
		};

		class MeshManager;

		class Mesh
		{
		public:
			Mesh() = default;
			~Mesh();

			void SetName(const char* name) { m_name = name; }
			const char* GetName() const { return m_name; }

			void SetRawData(RawVertexData data) { m_vertices_raw_data = data; }
			RawVertexData& GetRawData() { return m_vertices_raw_data; }
			const RawVertexData& GetRawData() const { return m_vertices_raw_data; }

			void SetVertexStart(u32 vertex_start) { m_allocation.vertex_start = vertex_start; }
			u32 GetVertexStart() const { return m_allocation.vertex_start; }

			void SetFirstIndex(u32 first_index) { m_allocation.first_index = first_index; }
			u32 GetFirstIndex() const { return m_allocation.first_index; }

			void SetIndexCount(u32 index_count) { m_allocation.index_count = index_count; }
			u32 GetIndexCount() const { return m_allocation.index_count; }

			void SetMaterialInfo(MaterialInfo* info) { m_material_info = info; }
			MaterialInfo* GetMaterialInfo() { return m_material_info; }

			void SetModelMatrix(glm::mat4 model_matrix) { m_model_matrix = model_matrix; }
			glm::mat4 GetModelMatrix() const { return m_model_matrix; }
			glm::mat4& GetModelMatrixRef() { return m_model_matrix; }

		private:
			const char* m_name;
			
			RawVertexData m_vertices_raw_data;

			BufferAllocation m_allocation;

			glm::mat4 m_model_matrix;

			MaterialInfo* m_material_info;
		};

	} // namespace graphics

} // namespace hellengine