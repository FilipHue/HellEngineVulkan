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

    m_texture_icon_scene = TextureManager::GetInstance()->CreateTexture2D(
        "ICON_SCENE",
        FileManager::ReadFile(CONCAT_PATHS(EDITOR_TEXTURES_PATH, "icon_scene.png"))
    );
    m_icon_scene = ImGui_ImplVulkan_AddTexture(
        m_texture_icon_scene->GetSampler(),
        m_texture_icon_scene->GetImageView(),
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    );

    m_texture_icon_entity = TextureManager::GetInstance()->CreateTexture2D(
        "ICON_ENTITY",
        FileManager::ReadFile(CONCAT_PATHS(EDITOR_TEXTURES_PATH, "icon_entity.png"))
    );
    m_icon_entity = ImGui_ImplVulkan_AddTexture(
        m_texture_icon_entity->GetSampler(),
        m_texture_icon_entity->GetImageView(),
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    );

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
                SceneManager::GetInstance()->CreateScene("Untitled");

            if (ImGui::MenuItem("Load Scene"))
            {
            }

            ImGui::PopStyleVar();
            ImGui::EndPopup();
        }
        ImGui::PopStyleVar();

        return;
    }

    DrawSceneNode(m_active_scene->GetName());
    DrawUtilityPanel();

    if (ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup) &&
        ImGui::IsMouseClicked(ImGuiMouseButton_Left) &&
        !ImGui::IsAnyItemHovered())
    {
        m_selected_game_object = Entity{};
        m_inspector_panel->SetSelectedEntity(m_selected_game_object);
    }
}

void EditorHierarchy::End()
{
    ImGui::End();
}

void EditorHierarchy::SetSelectedGameObject(Entity entity)
{
	m_selected_game_object = entity;
	m_inspector_panel->SetSelectedEntity(m_selected_game_object);
}

void EditorHierarchy::DrawSceneNode(const std::string& scene_name)
{
    auto& hierarchy = m_active_scene->GetHierarchy();
    const auto& roots = hierarchy.GetRootNodes();

    ImGuiTreeNodeFlags root_flags =
        ImGuiTreeNodeFlags_DefaultOpen |
        ImGuiTreeNodeFlags_SpanAvailWidth |
        ImGuiTreeNodeFlags_OpenOnArrow;

    if (roots.empty())
        root_flags |= ImGuiTreeNodeFlags_Leaf;

    ImGui::Image((ImTextureID)m_icon_scene, m_icon_size);
    ImGui::SameLine();

    // Use scene UUID for stable tree id
    b8 open = ImGui::TreeNodeEx((void*)(u64)m_active_scene->GetUUID(), root_flags, "%s", scene_name.c_str());

    // Drop on scene root => make dropped entity a root (no parent)
    if (ImGui::BeginDragDropTarget())
    {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(ENTITY_PAYLOAD))
        {
            UUID dragged_id = UUID((u64)INVALID_ID);
            std::memcpy(&dragged_id, payload->Data, sizeof(UUID));

            Entity dragged = m_active_scene->GetEntity(dragged_id);
            if (dragged)
            {
                SceneManager::GetInstance()->GetActiveScene()->ReparentGameObject(dragged, NULL_ENTITY);
            }
        }
        ImGui::EndDragDropTarget();
    }

    if (open)
    {
        for (UUID id : roots)
        {
            if ((u64)id == (u64)INVALID_ID) continue;

            Entity e = m_active_scene->GetEntity(id);
            if (e) DrawEntityNode(e);
        }

        ImGui::TreePop();
    }
}

void EditorHierarchy::DrawEntityNode(Entity entity)
{
    auto& hierarchy = m_active_scene->GetHierarchy();

    UUID id = entity.GetComponent<IDComponent>().id;
    HE_ASSERT((u64)id != (u64)INVALID_ID, "Hierarchy UI: entity has INVALID_ID");

    // Stable ImGui ID
    ImGui::PushID((i32)(u64)id);

    TagComponent& tag = entity.GetComponent<TagComponent>();

    ImGuiTreeNodeFlags flags =
        ImGuiTreeNodeFlags_OpenOnArrow |
        ImGuiTreeNodeFlags_SpanAvailWidth |
        ImGuiTreeNodeFlags_DefaultOpen;

    if (m_selected_game_object == entity)
        flags |= ImGuiTreeNodeFlags_Selected;

    // Leaf if no children
    if ((u64)hierarchy.GetFirstChild(id) == (u64)INVALID_ID)
        flags |= ImGuiTreeNodeFlags_Leaf;

    ImGui::Image((ImTextureID)m_icon_entity, m_icon_size);
    ImGui::SameLine();

    b8 open = ImGui::TreeNodeEx((void*)(u64)id, flags, "%s", tag.tag.c_str());

    if (ImGui::IsItemClicked(ImGuiMouseButton_Left) || ImGui::IsItemClicked(ImGuiMouseButton_Right))
    {
        m_selected_game_object = entity;
        m_inspector_panel->SetSelectedEntity(m_selected_game_object);
    }

    // Drag source payload is UUID (NOT entt::entity)
    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
    {
        ImGui::SetDragDropPayload(ENTITY_PAYLOAD, &id, sizeof(UUID));
        ImGui::TextUnformatted(tag.tag.c_str());
        ImGui::EndDragDropSource();
    }

    // Drop target: reparent dragged under this entity (cycle-safe)
    if (ImGui::BeginDragDropTarget())
    {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(ENTITY_PAYLOAD))
        {
            UUID dragged_id = UUID((u64)INVALID_ID);
            std::memcpy(&dragged_id, payload->Data, sizeof(UUID));

            if ((u64)dragged_id != (u64)INVALID_ID && dragged_id != id)
            {
                // Illegal if this target is inside dragged subtree:
                // i.e. target is descendant of dragged
                if (!hierarchy.IsDescendant(id, dragged_id))
                {
                    Entity dragged = m_active_scene->GetEntity(dragged_id);
                    if (dragged)
                        SceneManager::GetInstance()->GetActiveScene()->ReparentGameObject(dragged, entity);
                }
            }
        }
        ImGui::EndDragDropTarget();
    }

    if (open)
    {
        UUID child = hierarchy.GetFirstChild(id);
        while ((u64)child != (u64)INVALID_ID)
        {
            Entity child_entity = m_active_scene->GetEntity(child);
            if (child_entity) DrawEntityNode(child_entity);

            child = hierarchy.GetNextSibling(child);
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

        if (ImGui::MenuItem("Cut", "Ctrl + X", nullptr, m_selected_game_object != NULL_ENTITY)) {}
        if (ImGui::MenuItem("Copy", "Ctrl + C", nullptr, m_selected_game_object != NULL_ENTITY)) {}
        if (ImGui::MenuItem("Paste", "Ctrl + V", nullptr, m_selected_game_object != NULL_ENTITY)) {}
        if (ImGui::MenuItem("Rename", "Ctrl + R", nullptr, m_selected_game_object != NULL_ENTITY)) {}
        if (ImGui::MenuItem("Duplicate", "Ctrl + D", nullptr, m_selected_game_object != NULL_ENTITY)) {}

        if (ImGui::MenuItem("Delete", "Del", nullptr, m_selected_game_object != NULL_ENTITY))
        {
            Scene* scene = SceneManager::GetInstance()->GetActiveScene();
            if (!scene || !m_selected_game_object)
                return;

            UUID root = m_selected_game_object.GetComponent<IDComponent>().id;
            auto& h = scene->GetHierarchy();

            std::vector<UUID> stack;
            stack.push_back(root);

            while (!stack.empty())
            {
                UUID cur = stack.back();
                stack.pop_back();

                MeshManager::GetInstance()->RemoveMeshInstance(cur);

                UUID child = h.GetFirstChild(cur);
                while ((u64)child != (u64)INVALID_ID)
                {
                    stack.push_back(child);
                    child = h.GetNextSibling(child);
                }
            }

            scene->DestroyGameObject(m_selected_game_object);

            m_selected_game_object = NULL_ENTITY;
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
