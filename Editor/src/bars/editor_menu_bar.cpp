#include "editor_menu_bar.h"

void EditorMenuBar::Init(EditorHierarchy* hierarchy, EditorSettings* settings)
{
	m_hierarchy_panel = hierarchy;
	m_settings = settings;
}

void EditorMenuBar::Draw()
{
	ImGuiStyle& style = ImGui::GetStyle();

	ImGui::PushStyleVar(
		ImGuiStyleVar_ItemSpacing,
		ImVec2(
			style.ItemSpacing.x * MENU_BAR_ITEM_SPACING_X_MULTIPLIER,
			style.ItemSpacing.y
		)
	);

	if (ImGui::BeginMenuBar())
	{
		FileMenu(style);
		EditMenu(style);
		GameObjectMenu(style);
		ComponentMenu(style);
		AssetsMenu(style);
		WindowMenu(style);
		HelpMenu(style);

		ImGui::EndMenuBar();
	}

	ImGui::PopStyleVar();

	ShowEditorSettings();
	ShowRenderSettings();
	ShowGISettings();
	ShowShadowSettings();
	ShowAboutWindow();
}

void EditorMenuBar::SetImportAssetCallback(const std::function<void()>& callback)
{
	m_import_asset_callback = callback;
}

void EditorMenuBar::SetCreateEmptyGameObjectCallback(const std::function<void()>& callback)
{
	m_create_empty_gameobject_callback = callback;
}

void EditorMenuBar::FileMenu(ImGuiStyle& style)
{
	if (ImGui::BeginMenu("File"))
	{
		MenuSpacing(style);

		if (ImGui::MenuItem("New Scene", "Ctrl+N"))
		{
			SceneManager::GetInstance()->CreateScene("Untitled");
		}

		if (ImGui::MenuItem("Open Scene...", "Ctrl+O"))
		{
			File file = FileManager::OpenFile("Scene Files (*.yaml)\0*.yaml\0All Files (*.*)\0*.*\0");
			if (FileManager::Exists(file))
			{
				SceneManager::GetInstance()->LoadScene(file.GetAbsolutePath());
			}
		}

		ImGui::Separator();

		if (ImGui::MenuItem("Save", "Ctrl+S"))
		{
			SceneManager::GetInstance()->SaveScene(EDITOR_SCENE_PATH);
		}

		if (ImGui::MenuItem("Save As...", "Ctrl+Shift+S"))
		{
			SceneManager::GetInstance()->SaveSceneAs();
		}

		ImGui::Separator();

		if (ImGui::MenuItem("Exit"))
		{
		}

		ImGui::PopStyleVar();
		ImGui::EndMenu();
	}
}

void EditorMenuBar::EditMenu(ImGuiStyle& style)
{
	if (ImGui::BeginMenu("Edit"))
	{
		MenuSpacing(style);

		ImGui::MenuItem("Undo", "Ctrl+Z");
		ImGui::MenuItem("Redo", "Ctrl+Y");

		ImGui::Separator();

		ImGui::MenuItem("Duplicate", "Ctrl+D");
		ImGui::MenuItem("Delete", "Del");

		ImGui::Separator();

		ImGui::MenuItem("Project Settings");

		ImGui::PopStyleVar();
		ImGui::EndMenu();
	}
}

void EditorMenuBar::AssetsMenu(ImGuiStyle& style)
{
	if (ImGui::BeginMenu("Assets"))
	{
		MenuSpacing(style);

		if (ImGui::MenuItem("Import Model..."))
		{
			if (m_import_asset_callback)
			{
				m_import_asset_callback();
			}
			else
			{
				HE_CORE_WARN("Import Asset callback not set!");
			}
		}

		ImGui::SeparatorText("Create");

		ImGui::MenuItem("Material");
		ImGui::MenuItem("Shader");
		ImGui::MenuItem("Scene");

		ImGui::PopStyleVar();
		ImGui::EndMenu();
	}
}

void EditorMenuBar::GameObjectMenu(ImGuiStyle& style)
{
	if (ImGui::BeginMenu("GameObject"))
	{
		MenuSpacing(style);

		if (ImGui::MenuItem("Create Empty", "Ctrl+Shift+N"))
		{
			if (m_create_empty_gameobject_callback)
			{
				m_create_empty_gameobject_callback();
			}
			else
			{
				HE_CORE_WARN("Create Empty GameObject callback not set!");
			}
		}

		ImGui::SeparatorText("3D Objects");

		ImGui::MenuItem("Cube");
		ImGui::MenuItem("Sphere");
		ImGui::MenuItem("Plane");

		ImGui::SeparatorText("Lights");

		ImGui::MenuItem("Directional Light");
		ImGui::MenuItem("Point Light");
		ImGui::MenuItem("Spot Light");

		ImGui::PopStyleVar();
		ImGui::EndMenu();
	}
}

void EditorMenuBar::ComponentMenu(ImGuiStyle& style)
{
	if (ImGui::BeginMenu("Component"))
	{
		MenuSpacing(style);

		Entity selected = m_hierarchy_panel->GetSelectedGameObject();

		const bool has_selection = selected != NULL_ENTITY;

		ImGui::BeginDisabled(!has_selection);

		if (ImGui::MenuItem("Mesh Filter"))
		{
			selected.AddComponent<MeshFilterComponent>();
		}

		ImGui::EndDisabled();

		ImGui::PopStyleVar();
		ImGui::EndMenu();
	}
}

void EditorMenuBar::WindowMenu(ImGuiStyle& style)
{
	if (ImGui::BeginMenu("Window"))
	{
		MenuSpacing(style);

		ImGui::MenuItem("Hierarchy", nullptr, &m_settings->window_settings.show_hierarchy);
		ImGui::MenuItem("Inspector", nullptr, &m_settings->window_settings.show_inspector);

		ImGui::SeparatorText("Editor");

		ImGui::MenuItem("Editor Settings", nullptr, &m_settings->window_settings.show_editor_settings);

		ImGui::SeparatorText("Rendering");

		ImGui::MenuItem("Render Settings", nullptr, &m_settings->window_settings.show_render_settings);
		ImGui::MenuItem("GI Settings", nullptr, &m_settings->window_settings.show_gi_settings);
		ImGui::MenuItem("Shadow Settings", nullptr, &m_settings->window_settings.show_shadow_settings);

		ImGui::PopStyleVar();
		ImGui::EndMenu();
	}
}

void EditorMenuBar::HelpMenu(ImGuiStyle& style)
{
	if (ImGui::BeginMenu("Help"))
	{
		MenuSpacing(style);

		if (ImGui::MenuItem("Documentation"))
		{
		}

		if (ImGui::MenuItem("Keyboard Shortcuts"))
		{
		}

		ImGui::Separator();

		if (ImGui::MenuItem("Report Issue"))
		{
		}

		ImGui::SeparatorText("About");

		if (ImGui::MenuItem("About HellEngine"))
		{
			m_settings->window_settings.show_about_window = true;
		}

		ImGui::PopStyleVar();
		ImGui::EndMenu();
	}
}

void EditorMenuBar::ShowEditorSettings()
{
	if (!m_settings->window_settings.show_editor_settings)
	{
		return;
	}

	ImGuiWindowFlags flags = ImGuiWindowFlags_AlwaysAutoResize;

	const bool visible = ImGui::Begin(
		"Editor Settings",
		nullptr,
		flags
	);

	if (visible)
	{
		ImGui::SeparatorText("Scene View");

		ImGui::Checkbox("Show Grid", &m_settings->show_grid);
	}

	ImGui::End();
}

void EditorMenuBar::ShowRenderSettings()
{
	if (!m_settings->window_settings.show_render_settings)
	{
		return;
	}

	ImGuiWindowFlags flags = ImGuiWindowFlags_AlwaysAutoResize;

	const bool visible = ImGui::Begin(
		"Render Settings",
		nullptr,
		flags
	);

	if (visible)
	{
		ImGui::SeparatorText("Render Mode");

		const char* mode_names[] =
		{
			"Normal",
			"Wireframe",
			"UVs",
			"Normals",
			"Shadow Map"
		};

		i32 current_mode = static_cast<i32>(m_settings->render_mode);

		if (ImGui::Combo(
			"Mode",
			&current_mode,
			mode_names,
			EditorRenderMode_Count))
		{
			m_settings->render_mode =
				static_cast<EditorRenderMode>(current_mode);
		}
	}

	ImGui::End();
}

void EditorMenuBar::ShowGISettings()
{
	if (!m_settings->window_settings.show_gi_settings)
	{
		return;
	}

	ImGuiWindowFlags flags = ImGuiWindowFlags_AlwaysAutoResize;

	const bool visible = ImGui::Begin(
		"GI Settings",
		nullptr,
		flags
	);

	if (visible)
	{
		bool gi_enabled =
			static_cast<bool>(m_settings->gi_settings.enabled);

		if (ImGui::Checkbox("Enable GI", &gi_enabled))
		{
			m_settings->gi_settings.enabled = gi_enabled ? 1 : 0;
		}

		if (m_settings->gi_settings.enabled)
		{
			i32 sample_count =
				static_cast<i32>(m_settings->gi_settings.sample_count);

			if (ImGui::DragInt(
				"Samples",
				&sample_count,
				1.0f,
				GI_SAMPLE_COUNT_MIN,
				GI_SAMPLE_COUNT_MAX))
			{
				m_settings->gi_settings.sample_count =
					static_cast<u32>(sample_count);
			}

			ImGui::DragFloat(
				"Intensity##GI",
				&m_settings->gi_settings.intensity,
				GI_INTENSITY_STEP,
				GI_INTENSITY_MIN,
				GI_INTENSITY_MAX,
				"%.2f"
			);

			ImGui::DragFloat(
				"Ray Distance",
				&m_settings->gi_settings.ray_distance,
				GI_RAY_DISTANCE_STEP,
				GI_RAY_DISTANCE_MIN,
				GI_RAY_DISTANCE_MAX,
				"%.2f"
			);

			ImGui::DragFloat(
				"Thickness",
				&m_settings->gi_settings.thickness,
				GI_THICKNESS_STEP,
				GI_THICKNESS_MIN,
				GI_THICKNESS_MAX,
				"%.2f"
			);

			ImGui::DragFloat(
				"Falloff",
				&m_settings->gi_settings.falloff,
				GI_FALLOFF_STEP,
				GI_FALLOFF_MIN,
				GI_FALLOFF_MAX,
				"%.2f"
			);
		}
	}

	ImGui::End();
}

void EditorMenuBar::ShowShadowSettings()
{
	if (!m_settings->window_settings.show_shadow_settings)
	{
		return;
	}

	ImGuiWindowFlags flags = ImGuiWindowFlags_AlwaysAutoResize;

	const bool visible = ImGui::Begin(
		"Shadow Settings",
		nullptr,
		flags
	);

	if (visible)
	{
		bool shadows_enabled =
			static_cast<bool>(m_settings->shadow_settings.enabled);

		if (ImGui::Checkbox("Enable Shadows", &shadows_enabled))
		{
			m_settings->shadow_settings.enabled = shadows_enabled;
		}

		if (m_settings->shadow_settings.enabled)
		{
			const u32 shadow_map_sizes[] =
			{
				SHADOW_MAP_SIZE_512,
				SHADOW_MAP_SIZE_1024,
				SHADOW_MAP_SIZE_2048,
				SHADOW_MAP_SIZE_4096
			};

			const char* shadow_map_size_names[] =
			{
				"512",
				"1024",
				"2048",
				"4096"
			};

			i32 current_shadow_map_size_index = 0;

			for (i32 i = 0; i < IM_ARRAYSIZE(shadow_map_sizes); i++)
			{
				if (m_settings->shadow_settings.shadow_map_size == shadow_map_sizes[i])
				{
					current_shadow_map_size_index = i;
					break;
				}
			}

			if (ImGui::Combo(
				"Shadow Map Size",
				&current_shadow_map_size_index,
				shadow_map_size_names,
				IM_ARRAYSIZE(shadow_map_size_names)))
			{
				m_settings->shadow_settings.shadow_map_size =
					shadow_map_sizes[current_shadow_map_size_index];
			}

			ImGui::DragFloat(
				"Min Bias",
				&m_settings->shadow_settings.min_bias,
				SHADOW_MIN_BIAS_STEP,
				SHADOW_MIN_BIAS_MIN,
				SHADOW_MIN_BIAS_MAX,
				"%.4f"
			);

			ImGui::DragFloat(
				"Max Bias",
				&m_settings->shadow_settings.max_bias,
				SHADOW_MAX_BIAS_STEP,
				SHADOW_MAX_BIAS_MIN,
				SHADOW_MAX_BIAS_MAX,
				"%.4f"
			);

			m_settings->shadow_settings.max_bias = std::max(
				m_settings->shadow_settings.min_bias,
				m_settings->shadow_settings.max_bias
			);

			ImGui::DragFloat(
				"Normal Offset",
				&m_settings->shadow_settings.normal_offset,
				SHADOW_NORMAL_OFFSET_STEP,
				SHADOW_NORMAL_OFFSET_MIN,
				SHADOW_NORMAL_OFFSET_MAX,
				"%.2f"
			);

			i32 pcf_samples =
				static_cast<i32>(m_settings->shadow_settings.pcf_samples);

			if (ImGui::DragInt(
				"PCF Samples",
				&pcf_samples,
				SHADOW_PCF_SAMPLES_STEP,
				SHADOW_PCF_SAMPLES_MIN,
				SHADOW_PCF_SAMPLES_MAX))
			{
				m_settings->shadow_settings.pcf_samples =
					static_cast<u32>(pcf_samples);
			}

			ImGui::DragFloat(
				"Softness",
				&m_settings->shadow_settings.softness,
				SHADOW_SOFTNESS_STEP,
				SHADOW_SOFTNESS_MIN,
				SHADOW_SOFTNESS_MAX,
				"%.2f"
			);
		}
	}

	ImGui::End();
}

void EditorMenuBar::ShowAboutWindow()
{
	if (!m_settings->window_settings.show_about_window)
	{
		return;
	}

	ImGuiWindowFlags flags = ImGuiWindowFlags_AlwaysAutoResize;

	const bool visible = ImGui::Begin(
		"About HellEngine",
		&m_settings->window_settings.show_about_window,
		flags
	);

	if (visible)
	{
		ImGui::Text("HellEngine");
		ImGui::Separator();
		ImGui::Text("Custom real-time game engine editor.");
		ImGui::Text("Version: 0.1.0");
	}

	ImGui::End();
}

void EditorMenuBar::MenuSpacing(ImGuiStyle& style)
{
	ImGui::PushStyleVar(
		ImGuiStyleVar_ItemSpacing,
		ImVec2(
			style.ItemSpacing.x,
			style.ItemSpacing.y * MENU_ITEM_SPACING_Y_MULTIPLIER
		)
	);
}
