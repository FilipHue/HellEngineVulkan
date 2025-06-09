#pragma once

// Internal
#include <hellengine/core/uuid/uuid.h>
#include <hellengine/ecs/entity/entity.h>

namespace hellengine
{

	using namespace core;
	namespace ecs
	{

		class Scene
		{
		public:
			HE_API Scene();
			HE_API virtual ~Scene();

			HE_API void Create(std::string& name);
			HE_API void Destroy();

			/*HE_API Entity CreateEntity(const std::string& name = std::string());
			HE_API Entity CreateEntityWithUUID(UUID id, const std::string& name = std::string());
			HE_API void DestroyEntity(Entity& entity);*/

		private:
			//entt::registry m_registry;

			std::string m_name;

			//friend class Entity;
		};

	} // namespace ecs

} // namespace hellengine