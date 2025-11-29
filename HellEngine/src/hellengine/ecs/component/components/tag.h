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
			TagComponent(std::string tag) { tag = tag; }

			std::string tag;
		};

	} // namespace ecs

} // namespace hellengine