#pragma once

// Internal
#include <hellengine/ecs/shared.h>
#include <hellengine/core/uuid/uuid.h>

namespace hellengine
{

	using namespace core;
	namespace ecs
	{

		struct IDComponent
		{
			DEFAULT_ALL(IDComponent);
			IDComponent(UUID id) { id = id; }

			UUID id;
		};

 	} // namespace ecs

} // namespace hellengine