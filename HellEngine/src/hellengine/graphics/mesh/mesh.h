#pragma once

// Internal
#include "material.h"
#include <hellengine/core/core.h>
#include <hellengine/graphics/backend/vulkan_backend.h>

namespace hellengine
{

	namespace graphics
	{

		class Mesh
		{
		public:
			Mesh();
			~Mesh();

			void SetName(std::string name) { m_name = name; }
			std::string& GetName() { return m_name; }
			const std::string& GetName() const { return m_name; }

			void SetRawData(RawVertexData data) { m_verticies = data; }
			RawVertexData& GetRawData() { return m_verticies; }
			const RawVertexData& GetRawData() const { return m_verticies; }

			void SetIndices(const std::vector<u32>& indices) { m_indices = indices; }
			std::vector<u32>& GetIndices() { return m_indices; }
			const std::vector<u32>& GetIndices() const { return m_indices; }

			void SetMaterialInfo(MaterialInfo* info) { m_material_info = info; }
			MaterialInfo* GetMaterialInfo() { return m_material_info; }

		private:
			std::string m_name;
			RawVertexData m_verticies;
			std::vector<u32> m_indices;
			MaterialInfo* m_material_info;
		};

	} // namespace graphics

} // namespace hellengine