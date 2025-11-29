#pragma once

// Internal
#include <hellengine/ecs/shared.h>

// STL
#include <string>

namespace hellengine
{

	namespace ecs
	{

		struct MeshFilterComponent
		{
			DEFAULT_ALL(MeshFilterComponent);

			std::string mesh_id;
		};

	} // namespace ecs

} // namespace hellengine