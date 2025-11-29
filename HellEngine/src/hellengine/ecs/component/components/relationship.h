#pragma once

// Internal
#include <hellengine/ecs/shared.h>

// External
#include <entt/entt.hpp>

namespace hellengine
{
	namespace ecs
	{
		struct RelationshipComponent
		{
			DEFAULT_ALL(RelationshipComponent);

			entt::entity parent = entt::null;
			entt::entity first = entt::null;
			entt::entity next = entt::null;
			entt::entity prev = entt::null;
			entt::entity last = entt::null;

			u32 child_count = 0;
		};
	} // namespace ecs
} // namespace hellengine