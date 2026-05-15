#pragma once

// Internal
#include "hellengine/hellengine.h"

#include "../panels/editor_hierarchy.h"

using namespace hellengine;
using namespace core;
using namespace ecs;
using namespace graphics;
using namespace ui;
using namespace math;
using namespace resources;

enum EditorRenderMode
{
	EditorRenderMode_Normal = 0,
	EditorRenderMode_Wireframe = 1,
	EditorRenderMode_UVs = 2,
	EditorRenderMode_Normals = 3,
	EditorRenderMode_ShadowMap = 4, 

	EditorRenderMode_Count
};

struct EditorSettings
{
	b8 show_grid = true;

	EditorRenderMode render_mode = EditorRenderMode_Normal;
};

class EditorMenuBar
{
public:
	EditorMenuBar() = default;
	virtual ~EditorMenuBar() = default;

	void Init(EditorHierarchy* hierarchy);

	void Draw();

	EditorSettings* GetSettings() { return &m_settings; }

private:
	void FileMenu(ImGuiStyle& style);
	void AssetMenu(ImGuiStyle& style);
	void GameObjectMenu(ImGuiStyle& style);
	void ComponentMenu(ImGuiStyle& style);
	void SettingsMenu(ImGuiStyle& style);

	EditorSettings m_settings;
	EditorHierarchy* m_hierarchy_panel = nullptr;
};