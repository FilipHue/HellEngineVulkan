#pragma once

// Internal
#include <hellengine/ecs/shared.h>
#include <hellengine/ecs/component/components.h>

// External
#include <entt/entt.hpp>

namespace hellengine
{

	namespace ecs
	{

		class Entity
		{
		public:
			//HE_API Entity();
			//HE_API Entity(entt::entity handle, Scene* scene);
			//HE_API ~Entity();

			//HE_API void Create(entt::entity handle, Scene* scene);
			//HE_API void Destroy();

			//template<typename T, typename... Args>
			//T& AddComponent(Args&&... args)
			//{
			//	if (HasComponent<T>()) {
			//		HE_CORE_WARN("Entity already has the component {0}", GetComponentTypeToString<T>());
			//		
			//		return GetComponent<T>();
			//	}

			//	T& component = m_scene->m_registry.emplace<T>(m_handle, std::forward<Args>(args)...);

			//	return component;
			//}

			//template<typename T, typename... Args>
			//T& AddOrReplaceComponent(Args&&... args)
			//{
			//	return m_scene->m_registry.emplace_or_replace<T>(m_handle, std::forward<Args>(args)...);
			//}

			//template<typename T>
			//size_t RemoveComponent()
			//{
			//	if (!HasComponent<T>()) {
			//		HE_ECS_CRITICAL("Entity doesn't have the component {0}", GetComponentTypeToString<T>());
			//		EXIT(-1);
			//	}

			//	return m_scene->m_registry.remove<T>(m_handle);
			//}

			//template<typename T>
			//T& GetComponent()
			//{
			//	if (!HasComponent<T>()) {
			//		HE_ECS_CRITICAL("Entity doesn't have the component {0}", GetComponentTypeToString<T>());
			//		EXIT(-1);
			//	}

			//	return m_scene->m_registry.get<T>(m_handle);
			//}

			//template<typename T>
			//bool HasComponent()
			//{
			//	return m_scene->m_registry.all_of<T>(m_handle);
			//}

		private:
			//entt::entity m_handle;
		};

	} // namespace ecs

} // namespace hellengine