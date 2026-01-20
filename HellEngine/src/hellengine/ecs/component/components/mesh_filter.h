#pragma once

// Internal
#include <hellengine/ecs/shared.h>

// STL
#include <string>

namespace hellengine
{

	namespace graphics
	{
		class Mesh;
	}

	using namespace graphics;
	namespace ecs
	{

		struct MeshFilterComponent
		{
			DEFAULT_ALL(MeshFilterComponent);

			Mesh* mesh = nullptr;
		};

	} // namespace ecs

} // namespace hellengine