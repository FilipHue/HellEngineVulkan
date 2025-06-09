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
			TagComponent(std::string tag) { m_tag = tag; }

			std::string m_tag;
		};

	} // namespace ecs

} // namespace hellengine