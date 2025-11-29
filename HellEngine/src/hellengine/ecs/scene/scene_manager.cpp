#include "hepch.h"
#include "scene_manager.h"

// Internal
#include "hellengine/ecs/entity/entity.h"

namespace hellengine
{

	namespace ecs
	{

		void SceneManager::CreateScene(const std::string& name)
		{
			if (m_scenes.find(name) != m_scenes.end())
			{
				HE_CORE_WARN("Scene with name {0} already exists", name);
				return;
			}

			Scene* scene = new Scene();
			scene->Create(const_cast<std::string&>(name));

			m_scenes[name] = scene;
			m_active_scene = scene;
		}

		void SceneManager::DestroyScene(const std::string& name)
		{
			auto it = m_scenes.find(name);
			if (it != m_scenes.end())
			{
				if (m_active_scene == it->second)
				{
					m_active_scene = nullptr;
				}

				it->second->Destroy();
				delete it->second;
				m_scenes.erase(it);
			}
			else
			{
				HE_CORE_WARN("Scene with name {0} does not exist", name);
			}
		}

		Scene* SceneManager::GetScene(const std::string& name)
		{
			auto it = m_scenes.find(name);
			if (it != m_scenes.end())
			{
				return it->second;
			}
			else
			{
				HE_CORE_WARN("Scene with name {0} does not exist", name);
				return nullptr;
			}
		}

		b8 SceneManager::RenameScene(const std::string& old_name, const std::string& new_name)
		{
			auto it = m_scenes.find(old_name);
			if (it != m_scenes.end())
			{
				if (m_scenes.find(new_name) != m_scenes.end())
				{
					HE_CORE_WARN("Scene with name {0} already exists", new_name);
					return false;
				}

				Scene* scene = it->second;
				m_scenes.erase(it);
				scene->SetName(new_name);
				m_scenes[new_name] = scene;
				if (m_active_scene == scene)
				{
					m_active_scene = scene;
				}
				return true;
			}
			else
			{
				HE_CORE_WARN("Scene with name {0} does not exist", old_name);
				return false;
			}
		}

		SceneManager* SceneManager::GetInstance()
		{
			static SceneManager instance;
			return &instance;
		}

	} // namespace ecs

} // namespace hellengine
