#include "hepch.h"
#include "scene.h"

#pragma once

namespace hellengine
{

	namespace ecs
	{

		Scene::Scene()
		{
			NO_OP;
		}

		Scene::~Scene()
		{
			NO_OP;
		}

		void Scene::Create(std::string& name)
		{
			m_name = name;
		}

		void Scene::Destroy()
		{
		}

		//Entity Scene::CreateEntity(const std::string& name)
		//{
		//	return CreateEntityWithUUID(UUID(), name);
		//}

		//Entity Scene::CreateEntityWithUUID(UUID id, const std::string& name)
		//{
		//	Entity entity = { m_registry.create(), this };

		//	return entity;
		//}

		//void Scene::DestroyEntity(Entity& entity)
		//{
		//}

	} // namespace ecs

} // namespace hellengine