#pragma once

// Internal
#include "hellengine/hellengine.h"

#include "shared.h"

#include "bars/editor_menu_bar.h"
#include "panels/editor_hierarchy.h"
#include "panels/editor_inspector.h"
#include "panels/editor_viewport.h"

// External
#include <imguizmo/ImGuizmo.h>

using namespace hellengine;
using namespace core;
using namespace ecs;
using namespace graphics;
using namespace ui;
using namespace math;
using namespace resources;
using namespace tools;

ALIGN_AS(64) struct GlobalShaderData
{
	glm::mat4 proj;
	glm::mat4 view;
	glm::vec3 camera_position;
};

class Editor : public Application
{
public:
	Editor(ApplicationConfiguration* configuration);
	virtual ~Editor() = default;

	void Init() override;

	void OnProcessUpdate(f32 delta_time) override;

	void OnRenderBegin() override;
	void OnRenderUpdate() override;
	void OnRenderEnd() override;
	void OnUIRender() override;

	void Shutdown() override;

	b8 OnEventWindowClose(EventContext& event);
	b8 OnEventWindowResize(EventContext& event);
	b8 OnEventWindowFocus(EventContext& event);
	b8 OnEventWindowIconified(EventContext& event);
	b8 OnEventWindowMoved(EventContext& event);

	b8 OnEventKeyPressed(EventContext& event);
	b8 OnEventKeyReleased(EventContext& event);

	b8 OnEventMouseButtonPressed(EventContext& event);
	b8 OnEventMouseButtonReleased(EventContext& event);
	b8 OnEventMouseMoved(EventContext& event);
	b8 OnEventMouseScrolled(EventContext& event);

private:
	void CreateResources();
	void CreatePipelines();
	void CreateDescriptors();

	void CreateEditorUI();

	void DrawToSwapchain();

	void ShowGuizmo();

private:
	// Editor
	MultiProjectionCamera m_editor_camera;
	MultiProjectionController m_editor_camera_controller;

	glm::uvec2 m_viewport_last_size;

	ImGuizmo::OPERATION m_guizmo_operation = ImGuizmo::TRANSLATE;
	ImGuizmo::MODE m_guizmo_mode = ImGuizmo::LOCAL;

	// Editor UI
	EditorHierarchy* m_hierarchy_panel;
	EditorInspector* m_inspector_panel;
	EditorViewport* m_viewport_panel;
	EditorMenuBar* m_menu_bar;

	// PBR Pipeline
	DescriptorSet m_pbr_global_descriptor;
	UniformBuffer m_pbr_global_ubo;

	GlobalShaderData m_global_shader_data;
};

