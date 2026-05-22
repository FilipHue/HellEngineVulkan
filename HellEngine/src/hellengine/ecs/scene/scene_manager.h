#pragma once

// Internal
#include <hellengine/ecs/shared.h>
#include <hellengine/ecs/entity/entity.h>
#include <hellengine/ecs/scene/scene.h>
#include <hellengine/math/yaml_conversions.h>
#include <hellengine/graphics/managers/mesh_manager.h>
#include <hellengine/graphics/mesh/mesh.h>
#include <hellengine/resources/asset_manager.h>
#include <hellengine/resources/file_manager.h>

// External
#include <yaml-cpp/yaml.h>

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
			HE_API b8 Exists(const std::string& name) const;

			HE_API void LoadScene(const std::string& file_path);
			HE_API void SaveScene(const std::string& root_dir);
			HE_API void SaveSceneAs();

			HE_API void CreateEntitiesFromMeshes(Entity parent_entity = Entity());

			HE_API void SerializeScene(const std::string& name, const std::string& file_path);
			HE_API void DeserializeScene(const std::string& file_path);

			HE_API static SceneManager* GetInstance();

		private:
			void SerializeEntity(YAML::Node& entities_node, u32 entity_index, Entity entity, SceneHierarchy& hierarchy);
			void DeserializeEntity(const YAML::Node& entity_node, Scene* scene, Entity parent_entity = Entity());

			std::unordered_map<std::string, Scene*> m_scenes;
			Scene* m_active_scene = nullptr;
		};

	} // namespace ecs

} // namespace hellengine
