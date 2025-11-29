#pragma once

// Internal
#include <hellengine/ecs/shared.h>
#include <hellengine/ecs/entity/entity.h>
#include <hellengine/ecs/scene/scene.h>

namespace hellengine
{

	namespace ecs
	{

		class SceneManager
		{
		public:
			HE_API SceneManager() = default;
			HE_API ~SceneManager() = default;

			HE_API void CreateScene(const std::string& name);
			HE_API void DestroyScene(const std::string& name);
			HE_API Scene* GetScene(const std::string& name);
			HE_API Scene* GetActiveScene() const { return m_active_scene; }

			HE_API b8 RenameScene(const std::string& old_name, const std::string& new_name);

			HE_API static SceneManager* GetInstance();

		private:
			std::unordered_map<std::string, Scene*> m_scenes;
			Scene* m_active_scene = nullptr;
		};

	} // namespace ecs

} // namespace hellengine