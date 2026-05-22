#pragma once

// Internal
#include <hellengine/ecs/shared.h>

namespace hellengine
{

	namespace ecs
	{

		struct TagComponent
		{
			DEFAULT_ALL(TagComponent);

			std::string tag;
		};

	} // namespace ecs

} // namespace hellengine
