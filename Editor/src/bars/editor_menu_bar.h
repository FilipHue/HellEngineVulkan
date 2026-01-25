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

class EditorMenuBar
{
public:
	EditorMenuBar() = default;
	virtual ~EditorMenuBar() = default;

	void Init(EditorHierarchy* hierarchy);

	void Draw();

private:
	void FileMenu(ImGuiStyle& style);
	void AssetMenu(ImGuiStyle& style);
	void GameObjectMenu(ImGuiStyle& style);
	void ComponentMenu(ImGuiStyle& style);

	EditorHierarchy* m_hierarchy_panel = nullptr;
};