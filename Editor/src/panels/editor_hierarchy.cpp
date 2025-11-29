#include "editor_hierarchy.h"

// Internal
#include "hellengine/ecs/entity/entity.h"
#include "hellengine/ecs/scene/scene_manager.h"

EditorHierarchy::EditorHierarchy() : Panel("Hierarchy")
{
	NO_OP;
}

void EditorHierarchy::Init(EditorInspector* inspector)
{
	m_inspector_panel = inspector;

	m_active_scene = SceneManager::GetInstance()->GetActiveScene();
	m_selected_game_object = NULL_ENTITY;
	m_renaming = false;

	m_texture_icon_scene = TextureManager::GetInstance()->CreateTexture2D("ICON_SCENE", FileManager::ReadFile(CONCAT_PATHS(EDITOR_TEXTURES_PATH, "icon_scene.png")));
	m_icon_scene = ImGui_ImplVulkan_AddTexture(m_texture_icon_scene->GetSampler(), m_texture_icon_scene->GetImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	m_texture_icon_entity = TextureManager::GetInstance()->CreateTexture2D("ICON_ENTITY", FileManager::ReadFile(CONCAT_PATHS(EDITOR_TEXTURES_PATH, "icon_entity.png")));
	m_icon_entity = ImGui_ImplVulkan_AddTexture(m_texture_icon_entity->GetSampler(), m_texture_icon_entity->GetImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	m_icon_size = ImVec2(16.0f, 16.0f);
	m_editor_panel_size = ImVec2(300.0f, 0.0f);
}

b8 EditorHierarchy::Begin()
{
	if (!ImGui::Begin(m_name.c_str()))
	{
		ImGui::End();
		return false;
	}

	return true;
}

void EditorHierarchy::Draw()
{
	m_active_scene = SceneManager::GetInstance()->GetActiveScene();
	if (!m_active_scene)
	{
		ImGui::TextDisabled("< NO ACTIVE SCENE >");

		ImGui::SetNextWindowSize(m_editor_panel_size, ImGuiCond_Always);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(5.0f, 8.0f));
		if (ImGui::BeginPopupContextWindow(nullptr, ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems))
		{
			ImGuiStyle& style = ImGui::GetStyle();
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { style.ItemSpacing.x, style.ItemSpacing.y * 3.0f });
			if (ImGui::MenuItem("Create Scene"))
			{
				SceneManager::GetInstance()->CreateScene("Untitled");
			}

			if (ImGui::MenuItem("Load Scene"))
			{
			}

			ImGui::PopStyleVar();
			ImGui::EndPopup();
		}
		ImGui::PopStyleVar();
	}
	else
	{
		DrawSceneNode(m_active_scene->GetName());

		DrawUtilityPanel();

		if (ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup) && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ImGui::IsAnyItemHovered())
		{
			m_selected_game_object = Entity{};
			m_inspector_panel->SetSelectedEntity(m_selected_game_object);
		}
	}
}

void EditorHierarchy::End()
{
	ImGui::End();
}

void EditorHierarchy::DrawSceneNode(const std::string& scene_name)
{
	ImGuiTreeNodeFlags root_flags =
		ImGuiTreeNodeFlags_DefaultOpen |
		ImGuiTreeNodeFlags_SpanAvailWidth |
		ImGuiTreeNodeFlags_OpenOnArrow;

	const auto& view = m_active_scene->GetRegistry().view<IDComponent>();
	if (view.empty())
	{
		root_flags |= ImGuiTreeNodeFlags_Leaf;
	}

	ImGui::Image((ImTextureID)m_icon_scene, m_icon_size);
	ImGui::SameLine();

	b8 open = ImGui::TreeNodeEx(m_active_scene->GetUUID(), root_flags, "%s", scene_name.c_str());

	if (ImGui::BeginDragDropTarget())
	{
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(ENTITY_PAYLOAD))
		{
			entt::entity dragged_entity;
			std::memcpy(&dragged_entity, payload->Data, sizeof(dragged_entity));

			SceneManager::GetInstance()->GetActiveScene()->ReparentGameObject(
				Entity{ dragged_entity, m_active_scene },
				NULL_ENTITY
			);
		}
		ImGui::EndDragDropTarget();
	}

	if (open)
	{
		auto entities = m_active_scene->GetRegistry().storage<entt::entity>().each();
		for (const auto& [e] : entities)
		{
			Entity entity(e, m_active_scene);
			const auto& rel = entity.GetComponent<RelationshipComponent>();
			if (rel.parent == entt::null)
			{
				DrawEntityNode(entity);
			}
		}

		ImGui::TreePop();
	}
}

void EditorHierarchy::DrawEntityNode(Entity entity)
{
	ImGui::PushID((u32)entity);

	TagComponent& tag = entity.GetComponent<TagComponent>();
	ImGuiTreeNodeFlags flags = 
		ImGuiTreeNodeFlags_OpenOnArrow |
		ImGuiTreeNodeFlags_SpanAvailWidth | 
		ImGuiTreeNodeFlags_DefaultOpen;

	if (m_selected_game_object == entity)
	{
		flags |= ImGuiTreeNodeFlags_Selected;
	}

	if (entity.GetComponent<RelationshipComponent>().first == entt::null)
	{
		flags |= ImGuiTreeNodeFlags_Leaf;
	}

	ImGui::Image((ImTextureID)m_icon_entity, m_icon_size);
	ImGui::SameLine();
	b8 open = ImGui::TreeNodeEx("", flags, tag.tag.c_str());

	if (ImGui::IsItemClicked(ImGuiMouseButton_Left) || ImGui::IsItemClicked(ImGuiMouseButton_Right))
	{
		m_selected_game_object = entity;
		m_inspector_panel->SetSelectedEntity(m_selected_game_object);
	}

	if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
	{
		entt::entity handle = entity.GetHandle();
		ImGui::SetDragDropPayload(ENTITY_PAYLOAD, &handle, sizeof(handle));
		ImGui::TextUnformatted(tag.tag.c_str());
		ImGui::EndDragDropSource();
	}

	if (ImGui::BeginDragDropTarget())
	{
		if (auto* payload = ImGui::AcceptDragDropPayload(ENTITY_PAYLOAD))
		{
			entt::entity draggedHandle;
			std::memcpy(&draggedHandle, payload->Data, sizeof(draggedHandle));

			if (draggedHandle != entity.GetHandle() && !IsDescendant(entity, draggedHandle))
			{
				Entity dragged(draggedHandle, m_active_scene);
				SceneManager::GetInstance()->GetActiveScene()->ReparentGameObject(dragged, entity);
			}
		}
		ImGui::EndDragDropTarget();
	}

	if (open)
	{
		RelationshipComponent& relationship = entity.GetComponent<RelationshipComponent>();
		Entity child = { relationship.first, m_active_scene };
		while (child.GetHandle() != entt::null)
		{
			DrawEntityNode(child);
			child = { child.GetComponent<RelationshipComponent>().next, m_active_scene };
		}

		ImGui::TreePop();
	}

	ImGui::PopID();
}

void EditorHierarchy::DrawUtilityPanel()
{
	ImGui::SetNextWindowSize(m_editor_panel_size, ImGuiCond_Always);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(5.0f, 8.0f));
	if (ImGui::BeginPopupContextWindow(nullptr, ImGuiPopupFlags_MouseButtonRight))
	{
		ImGuiStyle& style = ImGui::GetStyle();
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { style.ItemSpacing.x, style.ItemSpacing.y * 3.0f });

		if (ImGui::MenuItem("Cut", "Ctrl + X", nullptr, m_selected_game_object != NULL_ENTITY))
		{
		}

		if (ImGui::MenuItem("Copy", "Ctrl + C", nullptr, m_selected_game_object != NULL_ENTITY))
		{
		}

		if (ImGui::MenuItem("Paste", "Ctrl + V", nullptr, m_selected_game_object != NULL_ENTITY))
		{
		}

		if (ImGui::MenuItem("Rename", nullptr, nullptr, m_selected_game_object != NULL_ENTITY))
		{
		}

		if (ImGui::MenuItem("Duplicate", "Ctrl + D", nullptr, m_selected_game_object != NULL_ENTITY))
		{
		}

		if (ImGui::MenuItem("Delete", "Del", nullptr, m_selected_game_object != NULL_ENTITY))
		{
			SceneManager::GetInstance()->GetActiveScene()->DestroyGameObject(m_selected_game_object);
			m_selected_game_object = Entity{};
			m_inspector_panel->SetSelectedEntity(m_selected_game_object);
		}

		ImGui::Separator();

		if (ImGui::MenuItem("Create Empty"))
		{
			SceneManager::GetInstance()->GetActiveScene()->CreateGameObject("GameObject", m_selected_game_object);
		}

		ImGui::PopStyleVar();
		ImGui::EndPopup();
	}
	ImGui::PopStyleVar();

}

b8 EditorHierarchy::IsDescendant(Entity parent, entt::entity possibleChild)
{
	auto& reg = m_active_scene->GetRegistry();
	entt::entity current = parent.GetComponent<RelationshipComponent>().first;
	while (current != entt::null)
	{
		if (current == possibleChild) return true;
		current = reg.get<RelationshipComponent>(current).next;
	}
	return false;
}
