#include "hepch.h"
#include "scene_manager.h"

// Internal
#include "hellengine/ecs/entity/entity.h"

// External
#include <yaml-cpp/yaml.h>

namespace hellengine
{

	using namespace resources;
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

		void SceneManager::LoadScene(const std::string& file_path)
		{
			if (!FileManager::Exists(file_path))
			{
				HE_CORE_WARN("Cannot load scene: File {0} does not exist", file_path);
				return;
			}
			DeserializeScene(file_path);
		}

		void SceneManager::SaveScene(const std::string& root_dir)
		{
			if (m_active_scene)
			{
				SerializeScene(m_active_scene->GetName(), CONCAT_PATHS(root_dir, m_active_scene->GetName() + ".yaml"));
			}
			else
			{
				HE_CORE_WARN("Cannot save: No active scene");
			}
		}

		void SceneManager::SaveSceneAs()
		{
			if (m_active_scene)
			{
				std::string file_path = FileManager::SaveFile("YAML Scene (*.yaml)\0*.yaml\0");
				if (!file_path.empty())
				{
					SerializeScene(m_active_scene->GetName(), file_path);
				}
				else
				{
					HE_CORE_WARN("Save cancelled by user");
				}
			}
			else
			{
				HE_CORE_WARN("Cannot save: No active scene");
			}
		}

		void SceneManager::SerializeScene(const std::string& name, const std::string& file_path)
		{
			Scene* scene = GetScene(name);
			if (!scene)
			{
				HE_CORE_WARN("Cannot serialize: Scene with name {0} does not exist", name);
				return;
			}

			YAML::Node root;

			// Scene metadata
			root["scene"]["name"] = scene->GetName();
			root["scene"]["uuid"] = (u64)scene->GetUUID();

			// Get hierarchy and registry
			SceneHierarchy& hierarchy = const_cast<SceneHierarchy&>(scene->GetHierarchy());
			SceneRegistry& registry = scene->GetRegistry();

			YAML::Node entities_node;
			u32 entity_index = 0;

			// Find and serialize all root entities (entities with no parent)
			auto view = registry.view<IDComponent>();
			for (auto entity_handle : view)
			{
				Entity entity(entity_handle, scene);
				IDComponent& id_comp = entity.GetComponent<IDComponent>();

				// Only serialize root entities; children will be serialized through hierarchy
				if (!hierarchy.Exists(id_comp.id) || (u64)hierarchy.GetParent(id_comp.id) == INVALID_ID)
				{
					SerializeEntity(entities_node, entity_index, entity, hierarchy);
					entity_index++;
				}
			}

			root["entities"] = entities_node;

			FileManager::WriteFile(file_path, YAML::Dump(root));
		}

		void SceneManager::SerializeEntity(YAML::Node& entities_node, u32 entity_index, Entity entity, SceneHierarchy& hierarchy)
		{
			YAML::Node entity_node;

			Scene* scene = entity.GetScene();
			SceneRegistry& registry = scene->GetRegistry();

			// Serialize ID Component
			if (entity.HasComponent<IDComponent>())
			{
				IDComponent& id_comp = entity.GetComponent<IDComponent>();
				entity_node["id"] = (u64)id_comp.id;
			}

			// Serialize Tag Component
			if (entity.HasComponent<TagComponent>())
			{
				TagComponent& tag_comp = entity.GetComponent<TagComponent>();
				entity_node["tag"] = tag_comp.tag;
			}

			// Serialize Transform Component
			if (entity.HasComponent<TransformComponent>())
			{
				TransformComponent& transform = entity.GetComponent<TransformComponent>();
				entity_node["transform"]["position"] = transform.local_position;
				entity_node["transform"]["rotation"] = transform.local_rotation;
				entity_node["transform"]["scale"] = transform.local_scale;
			}

			// Serialize Light Component
			if (entity.HasComponent<LightComponent>())
			{
				LightComponent& light = entity.GetComponent<LightComponent>();
				entity_node["light"]["type"] = (i32)light.type;
				entity_node["light"]["color"] = light.color;
				entity_node["light"]["intensity"] = light.intensity;
				entity_node["light"]["range"] = light.range;
				entity_node["light"]["attenuation"] = light.attenuation;
				entity_node["light"]["inner_cone_angle"] = light.inner_cone_angle;
				entity_node["light"]["outer_cone_angle"] = light.outer_cone_angle;
				entity_node["light"]["enabled"] = light.enabled;
				entity_node["light"]["cast_shadows"] = light.cast_shadows;
			}

			// Serialize MeshFilter Component
			if (entity.HasComponent<MeshFilterComponent>())
			{
				MeshFilterComponent& mesh_filter = entity.GetComponent<MeshFilterComponent>();
				if (mesh_filter.mesh != nullptr)
				{
					entity_node["mesh_filter"]["mesh_name"] = mesh_filter.mesh->GetName();
					entity_node["mesh_filter"]["mesh_path"] = MeshManager::GetInstance()->GetMeshPath(mesh_filter.mesh->GetName());
				}
			}

			// Recursively serialize children
			YAML::Node children_node;
			u32 child_index = 0;
			IDComponent& id_comp = entity.GetComponent<IDComponent>();

			UUID first_child_id = hierarchy.GetFirstChild(id_comp.id);
			if ((u64)first_child_id != INVALID_ID)
			{
				Entity child_entity = scene->GetEntity(first_child_id);
				if (child_entity)
				{
					SerializeEntity(children_node, child_index, child_entity, hierarchy);
					child_index++;

					// Continue with siblings
					UUID next_sibling_id = hierarchy.GetNextSibling(first_child_id);
					while ((u64)next_sibling_id != INVALID_ID)
					{
						Entity sibling_entity = scene->GetEntity(next_sibling_id);
						if (sibling_entity)
						{
							SerializeEntity(children_node, child_index, sibling_entity, hierarchy);
							child_index++;
						}
						next_sibling_id = hierarchy.GetNextSibling(next_sibling_id);
					}
				}
			}

			if (child_index > 0)
			{
				entity_node["children"] = children_node;
			}

			entities_node[entity_index] = entity_node;
		}

		void SceneManager::DeserializeScene(const std::string& file_path)
		{
			if (!FileManager::Exists(file_path))
			{
				HE_CORE_ERROR("Scene file does not exist: {0}", file_path);
				return;
			}

			YAML::Node root = YAML::LoadFile(file_path);

			if (!root["scene"])
			{
				HE_CORE_ERROR("Invalid scene file format: missing 'scene' node in {0}", file_path);
				return;
			}

			// Get scene metadata
			std::string scene_name = root["scene"]["name"].as<std::string>("Untitled Scene");
			u64 scene_uuid = root["scene"]["uuid"].as<u64>(0);

			// Create or get the scene
			CreateScene(scene_name);
			Scene* scene = GetScene(scene_name);
			if (!scene)
			{
				HE_CORE_ERROR("Failed to create scene: {0}", scene_name);
				return;
			}

			// Deserialize entities
			if (root["entities"])
			{
				for (size_t i = 0; i < root["entities"].size(); ++i)
				{
					DeserializeEntity(root["entities"][i], scene);
				}
			}

			TextureType material_texture_types =
				TextureType_Diffuse |
				TextureType_Specular |
				TextureType_Ambient |
				TextureType_Emissive |
				TextureType_Height |
				TextureType_Normals |
				TextureType_Shininess |
				TextureType_Opacity |
				TextureType_Displacement |
				TextureType_Lightmap |
				TextureType_Reflection;
			MeshManager::GetInstance()->UploadToGpu(material_texture_types);

			HE_CORE_INFO("Scene {0} deserialized from {1}", scene_name, file_path);
		}

		void SceneManager::DeserializeEntity(const YAML::Node& entity_node, Scene* scene, Entity parent_entity)
		{
			if (!entity_node.IsMap())
			{
				HE_CORE_WARN("Invalid entity node format");
				return;
			}

			// Get UUID
			UUID entity_uuid = entity_node["id"].as<u64>(UUID::Generate());

			// Get Tag
			std::string tag = entity_node["tag"].as<std::string>("Entity");

			Entity entity = scene->CreateEntityWithUUID(entity_uuid, tag);
			scene->ReparentGameObject(entity, parent_entity);

			// Deserialize Transform Component
			if (entity_node["transform"])
			{
				TransformComponent& transform = entity.HasComponent<TransformComponent>() ? 
					entity.GetComponent<TransformComponent>() : 
					entity.AddComponent<TransformComponent>();

				if (entity_node["transform"]["position"])
					transform.local_position = entity_node["transform"]["position"].as<glm::vec3>();
				if (entity_node["transform"]["rotation"])
					transform.local_rotation = entity_node["transform"]["rotation"].as<glm::vec3>();
				if (entity_node["transform"]["scale"])
					transform.local_scale = entity_node["transform"]["scale"].as<glm::vec3>();

				transform.is_dirty = true;
			}

			// Deserialize Light Component
			if (entity_node["light"])
			{
				LightComponent& light = entity.HasComponent<LightComponent>() ? 
					entity.GetComponent<LightComponent>() : 
					entity.AddComponent<LightComponent>();

				if (entity_node["light"]["type"])
					light.type = static_cast<LightType>(entity_node["light"]["type"].as<i32>());
				if (entity_node["light"]["color"])
					light.color = entity_node["light"]["color"].as<glm::vec3>();
				if (entity_node["light"]["intensity"])
					light.intensity = entity_node["light"]["intensity"].as<float>();
				if (entity_node["light"]["range"])
					light.range = entity_node["light"]["range"].as<float>();
				if (entity_node["light"]["attenuation"])
					light.attenuation = entity_node["light"]["attenuation"].as<float>();
				if (entity_node["light"]["inner_cone_angle"])
					light.inner_cone_angle = entity_node["light"]["inner_cone_angle"].as<float>();
				if (entity_node["light"]["outer_cone_angle"])
					light.outer_cone_angle = entity_node["light"]["outer_cone_angle"].as<float>();
				if (entity_node["light"]["enabled"])
					light.enabled = entity_node["light"]["enabled"].as<b8>();
				if (entity_node["light"]["cast_shadows"])
					light.cast_shadows = entity_node["light"]["cast_shadows"].as<b8>();
			}

			// Deserialize MeshFilter Component
			if (entity_node["mesh_filter"])
			{
				MeshFilterComponent& mesh_filter = entity.HasComponent<MeshFilterComponent>() ? 
					entity.GetComponent<MeshFilterComponent>() : 
					entity.AddComponent<MeshFilterComponent>();

				Mesh* loaded_mesh = nullptr;

				if (entity_node["mesh_filter"]["mesh_name"])
				{
					std::string mesh_name = entity_node["mesh_filter"]["mesh_name"].as<std::string>();
					std::string mesh_path = entity_node["mesh_filter"]["mesh_path"].as<std::string>();
					static std::string last_path = "";
					if (MeshManager::GetInstance()->GetMeshPath(mesh_name) != last_path || last_path == "")
					{
						AssetManager::LoadModel(FileManager::ReadFile(mesh_path));

						last_path = MeshManager::GetInstance()->GetMeshPath(mesh_name);
					}

					MeshManager::GetInstance()->CreateMeshInstance(entity_uuid, MeshManager::GetInstance()->GetMeshByName(mesh_name));
					loaded_mesh = MeshManager::GetInstance()->GetMeshByName(mesh_name);
					if (loaded_mesh)
					{
						mesh_filter.mesh = loaded_mesh;
					}
				}
			}

			// Recursively deserialize children
			if (entity_node["children"])
			{
				for (size_t i = 0; i < entity_node["children"].size(); ++i)
				{
					DeserializeEntity(entity_node["children"][i], scene, entity);
				}
			}
		}

		void SceneManager::CreateEntitiesFromMeshes(Entity parent_entity)
		{
			if (!m_active_scene)
			{
				HE_CORE_ERROR("No active scene to create entities in");
				return;
			}

			std::vector<graphics::Mesh*> root_meshes = AssetManager::GetLoadedRootMeshes();

			if (root_meshes.empty())
			{
				HE_CORE_WARN("No root meshes found to create entities from");
				return;
			}

			// Create a root entity with the filename
			std::string root_name = AssetManager::GetLastLoadedFilename();
			if (root_name.empty())
			{
				root_name = "Model";
			}

			Entity root_entity = m_active_scene->CreateGameObject(root_name, parent_entity);
			HE_GRAPHICS_INFO("Created root entity: {0}", root_name);

			// Create entities for all root meshes under this parent
			for (graphics::Mesh* root_mesh : root_meshes)
			{
				if (root_mesh)
				{
					RecursiveCreateEntitiesFromMesh(root_mesh, root_entity);
				}
			}
		}

		void SceneManager::RecursiveCreateEntitiesFromMesh(graphics::Mesh* mesh, Entity parent_entity)
		{
			if (!mesh || !m_active_scene)
			{
				HE_CORE_WARN("RecursiveCreateEntitiesFromMesh: mesh or scene is null");
				return;
			}

			HE_GRAPHICS_INFO("Creating entity from mesh: {0}", mesh->GetName());

			// Create entity for this mesh
			Entity entity = m_active_scene->CreateGameObject(mesh->GetName(), parent_entity);

			// Add MeshFilterComponent
			entity.AddComponent<MeshFilterComponent>().mesh = mesh;

			// Create mesh instance for rendering
			UUID entity_uuid = entity.GetComponent<IDComponent>().id;
			MeshManager::GetInstance()->CreateMeshInstance(entity_uuid, mesh);

			HE_GRAPHICS_INFO("Created entity '{0}' for mesh with {1} vertices", mesh->GetName(), mesh->GetRawData().positions.size());

			// Recursively create entities for all child meshes
			const std::vector<graphics::Mesh*>& children = mesh->GetChildren();
			for (graphics::Mesh* child : children)
			{
				if (child)
				{
					RecursiveCreateEntitiesFromMesh(child, entity);
				}
			}

			HE_GRAPHICS_INFO("Mesh '{0}' has {1} children", mesh->GetName(), children.size());
		}

		SceneManager* SceneManager::GetInstance()
		{
			static SceneManager instance;
			return &instance;
		}

	} // namespace ecs

} // namespace hellengine
