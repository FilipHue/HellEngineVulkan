#pragma once

// Internal
#include <hellengine/ecs/shared.h>

namespace hellengine
{

	using namespace core;
	namespace ecs
	{

		class Entity;
		class Scene
		{
		public:
			Scene();
			virtual ~Scene();

			void Create(std::string& name);
			void Destroy();

			Entity CreateEntity(const std::string& name = std::string());
			Entity CreateEntityWithUUID(UUID id, const std::string& name = std::string());
			void DestroyEntity(Entity entity);

			Entity CreateGameObject(const std::string& name, Entity parent);
			void DestroyGameObject(Entity entity);
			void ReparentGameObject(Entity child, Entity parent);

			b8 IsValid(Entity entity) const;

			void UpdateTransforms();

			Entity GetEntity(std::string name);
			SceneRegistry& GetRegistry() { return m_registry; }

			std::string& GetName() { return m_name; }
			const std::string& GetName() const { return m_name; }
			void SetName(const std::string& name) { m_name = name; }

			UUID GetUUID() const { return m_uuid; }

		private:
			SceneRegistry m_registry;

			std::string m_name;
			UUID m_uuid;
		};

	} // namespace ecs

} // namespace hellengine