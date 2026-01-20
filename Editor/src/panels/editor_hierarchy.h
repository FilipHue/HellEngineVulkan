#pragma once

// Internal
#include "hellengine/hellengine.h"
#include "editor_inspector.h"

using namespace hellengine;
using namespace core;
using namespace ecs;
using namespace graphics;
using namespace ui;
using namespace math;
using namespace resources;

constexpr auto EDITOR_TEXTURES_PATH = "assets/editor/textures/";

constexpr auto ENTITY_PAYLOAD = "PYLD_ENTITY";

class EditorHierarchy : public Panel
{
public:
	EditorHierarchy();
	virtual ~EditorHierarchy() = default;

	void Init(EditorInspector* inspector);

	b8 Begin() override;
	void Draw() override;
	void End() override;

	void SetSelectedGameObject(Entity entity);
	Entity GetSelectedGameObject() const { return m_selected_game_object; }

private:
	void DrawSceneNode(const std::string& scene_name);
	void DrawEntityNode(Entity entity);
	void DrawUtilityPanel();

private:
	EditorInspector* m_inspector_panel;

	Scene* m_active_scene = nullptr;
	Entity m_selected_game_object;

	b8 m_renaming = false;

	VulkanTexture2D* m_texture_icon_scene;
	VulkanTexture2D* m_texture_icon_entity;
	void* m_icon_scene;
	void* m_icon_entity;

	ImVec2 m_icon_size;
	ImVec2 m_editor_panel_size;
};
