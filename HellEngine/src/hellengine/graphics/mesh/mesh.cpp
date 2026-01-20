#include "hepch.h"
#include "mesh.h"

namespace hellengine
{

	namespace graphics
	{

		Mesh::Mesh()
		{
			m_material_info = nullptr;
		}

		Mesh::~Mesh()
		{
			if (m_material_info)
			{
				delete m_material_info;
			}
		}

	} // namespace graphics

} // namespace hellengine