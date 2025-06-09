#include "hepch.h"
#include "mesh.h"

namespace hellengine
{
	namespace graphics
	{

		Mesh::~Mesh()
		{
			delete m_material_info;
		}

	} // namespace graphics

} // namespace hellengine