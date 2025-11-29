#include "hepch.h"
#include "entity.h"

namespace hellengine
{

	namespace ecs
	{

		Entity::Entity() : m_handle(entt::null), m_scene(nullptr)
		{
			NO_OP;
		}

		Entity::Entity(EntityId handle, Scene* scene) : m_handle(handle), m_scene(scene)
		{
			NO_OP;
		}

		Entity::~Entity()
		{
			NO_OP;
		}

	} // namespace ecs

} // namespace hellengine