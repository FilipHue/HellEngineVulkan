#pragma once

// Internal
#include <hellengine/ecs/shared.h>
#include <hellengine/ecs/component/components.h>
#include <hellengine/ecs/scene/scene.h>

namespace hellengine
{

	namespace ecs
	{

		class Entity
		{
		public:
			Entity();
			Entity(EntityId handle, Scene* scene);
			~Entity();

			entt::entity GetHandle() const { return m_handle; }
			Scene* GetScene() const { return m_scene; }

			template<typename T, typename... Args>
			T& AddComponent(Args&&... args)
			{
				if (HasComponent<T>()) {
					HE_CORE_WARN("Entity already has the component {0}", GetComponentTypeToString<T>());
					
					return GetComponent<T>();
				}

				T& component = m_scene->GetRegistry().emplace<T>(m_handle, std::forward<Args>(args)...);

				return component;
			}

			template<typename T, typename... Args>
			T& AddOrReplaceComponent(Args&&... args)
			{
				return m_scene->GetRegistry().emplace_or_replace<T>(m_handle, std::forward<Args>(args)...);
			}

			template<typename T>
			size_t RemoveComponent()
			{
				if (!HasComponent<T>()) {
					HE_ECS_CRITICAL("Entity doesn't have the component {0}", GetComponentTypeToString<T>());
					EXIT(-1);
				}

				return m_scene->GetRegistry().remove<T>(m_handle);
			}

			template<typename T>
			T& GetComponent()
			{
				if (!HasComponent<T>()) {
					HE_ECS_CRITICAL("Entity doesn't have the component {0}", GetComponentTypeToString<T>());
					EXIT(-1);
				}

				return m_scene->GetRegistry().get<T>(m_handle);
			}

			template<typename T>
			bool HasComponent()
			{
				return m_scene->GetRegistry().all_of<T>(m_handle);
			}

			bool operator==(const Entity& other) const { return m_handle == other.m_handle && m_scene == other.m_scene; }
			bool operator!=(const Entity& other) const { return !(*this == other); }
			operator bool() const { return m_handle != entt::null; }
			operator u32() const { return (u32)m_handle; }

			operator EntityId() const { return m_handle; }

		private:
			EntityId m_handle;
			Scene* m_scene;
		};

		INLINE static const Entity NULL_ENTITY = Entity();

	} // namespace ecs

} // namespace hellengine