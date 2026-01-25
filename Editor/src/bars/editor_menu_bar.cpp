#include "editor_menu_bar.h"

void EditorMenuBar::Init(EditorHierarchy* hierarchy)
{
	m_hierarchy_panel = hierarchy;
}

void EditorMenuBar::Draw()
{
	ImGuiStyle& style = ImGui::GetStyle();
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { style.ItemSpacing.x * 2.0f, style.ItemSpacing.y });

	if (ImGui::BeginMenuBar())
	{
		FileMenu(style);
		ImGui::SetNextWindowSize(ImVec2(400.0f, 0.0f), ImGuiCond_Always);
		AssetMenu(style);
		ImGui::SetNextWindowSize(ImVec2(400.0f, 0.0f), ImGuiCond_Always);
		GameObjectMenu(style);
		ImGui::SetNextWindowSize(ImVec2(400.0f, 0.0f), ImGuiCond_Always);
		ComponentMenu(style);

		ImGui::EndMenuBar();
	}

	ImGui::PopStyleVar();
}

void EditorMenuBar::FileMenu(ImGuiStyle& style)
{
	ImGui::SetNextWindowSize(ImVec2(400.0f, 0.0f), ImGuiCond_Always);
	if (ImGui::BeginMenu("File"))
	{
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(style.ItemSpacing.x, style.ItemSpacing.y * 3.0f));
		if (ImGui::MenuItem("	New Scene", "Ctrl + N"))
		{
			SceneManager::GetInstance()->CreateScene("Untitled");
		}

		if (ImGui::MenuItem("	Open Scene", "Ctrl + O"))
		{
		}

		ImGui::Separator();

		if (ImGui::MenuItem("	Save", "Ctrl + S"))
		{
		}

		if (ImGui::MenuItem("	Save As", "Ctrl + Shift + S"))
		{
		}

		ImGui::PopStyleVar();
		ImGui::EndMenu();
	}
}

void EditorMenuBar::AssetMenu(ImGuiStyle& style)
{
	if (ImGui::BeginMenu("Assets"))
	{
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(style.ItemSpacing.x, style.ItemSpacing.y * 3.0f));

		ImGui::Separator();

		if (ImGui::MenuItem("	Import New Asset"))
		{
			if (SceneManager::GetInstance()->GetActiveScene() == nullptr)
			{
				SceneManager::GetInstance()->CreateScene("Untitled");
			}

			File file = FileManager::OpenFile("Model Files (*.gltf)\0*.gltf\0Model Files (*.glb)\0*.glb\0All Files (*.*)\0*.*\0");
			if (FileManager::Exists(file.GetAbsolutePath()))
			{
				AssetManager::LoadModel(file);
				MeshManager::GetInstance()->UploadToGpu(TextureType_Diffuse | TextureType_Ambient | TextureType_Specular);
			}
		}

		ImGui::PopStyleVar();
		ImGui::EndMenu();
	}
}

void EditorMenuBar::GameObjectMenu(ImGuiStyle& style)
{
	if (ImGui::BeginMenu("GameObject"))
	{
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(style.ItemSpacing.x, style.ItemSpacing.y * 3.0f));
		if (ImGui::MenuItem("	Create Empty", "Ctrl + Shift + N"))
		{
			if (SceneManager::GetInstance()->GetActiveScene() == nullptr)
			{
				SceneManager::GetInstance()->CreateScene("Untitled");
			}

			SceneManager::GetInstance()->GetActiveScene()->CreateGameObject("GameObject", m_hierarchy_panel->GetSelectedGameObject());
		}

		ImGui::PopStyleVar();
		ImGui::EndMenu();
	}
}

void EditorMenuBar::ComponentMenu(ImGuiStyle& style)
{
	if (ImGui::BeginMenu("Component"))
	{
		if (ImGui::MenuItem("	Add", "Ctrl + Shift + A", nullptr, m_hierarchy_panel->GetSelectedGameObject() != NULL_ENTITY))
		{
		}

		ImGui::Separator();

		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(style.ItemSpacing.x, style.ItemSpacing.y * 3.0f));

		if (ImGui::BeginMenu("Mesh"))
		{
			Entity selected = m_hierarchy_panel->GetSelectedGameObject();
			const b8 hasSelection = (selected != NULL_ENTITY);

			b8 enabled = hasSelection && !selected.HasComponent<MeshFilterComponent>();
			if (ImGui::MenuItem("	Mesh Filter", nullptr, false, enabled))
			{
				selected.AddComponent<MeshFilterComponent>();
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndMenu();
		}

		ImGui::PopStyleVar();
		ImGui::EndMenu();
	}
}
