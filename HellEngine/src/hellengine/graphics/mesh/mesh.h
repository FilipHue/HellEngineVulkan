#pragma once

// Internal
#include "material.h"
#include <hellengine/core/core.h>
#include <hellengine/graphics/backend/vulkan_backend.h>

namespace hellengine
{

	namespace graphics
	{
		class MeshManager;

		class Mesh
		{
		public:
			Mesh() = default;
			~Mesh();

			void SetVertexStart(u32 vertex_start) { m_vertex_start = vertex_start; }
			u32 GetVertexStart() const { return m_vertex_start; }

			void SetFirstIndex(u32 first_index) { m_first_index = first_index; }
			u32 GetFirstIndex() const { return m_first_index; }

			void SetIndexCount(u32 index_count) { m_index_count = index_count; }
			u32 GetIndexCount() const { return m_index_count; }

			void SetMaterial(Material* material) { m_material = material; }
			Material* GetMaterial() { return m_material; }

			void SetModelMatrix(glm::mat4 model_matrix) { m_model_matrix = model_matrix; }
			glm::mat4 GetModelMatrix() const { return m_model_matrix; }
			glm::mat4& GetModelMatrixRef() { return m_model_matrix; }

		private:
			u32 m_vertex_start;
			u32 m_first_index;
			u32 m_index_count;

			glm::mat4 m_model_matrix;

			Material* m_material;
		};

	} // namespace graphics

} // namespace hellengine