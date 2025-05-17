#include "hepch.h"
#include "mesh.h"

namespace hellengine
{
	namespace graphics
	{

		Mesh::~Mesh()
		{
			delete m_material;
		}

	} // namespace graphics

} // namespace hellengine