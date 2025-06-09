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
			IDComponent(UUID id) { m_id = id; }

			UUID m_id;
		};

 	} // namespace ecs

} // namespace hellengine