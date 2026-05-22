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

			UUID id;
		};

 	} // namespace ecs

} // namespace hellengine
