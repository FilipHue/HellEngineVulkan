#pragma once

// Internal
#include "hellengine/hellengine.h"

#include "shared.h"
#include "types.h"

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

struct EditorSpecificData
{
	u32 show_debug_info = 0;
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
	void ImportAsset();
	void CreateEmptyGameObject();

	void CreateDefaultSettings();

	void CreateResources();
	void CreatePipelines();
	void CreateDescriptors();

	void CreateEditorUI();

	void DrawToSwapchain();

	void ShowGuizmo();

	void UpdateLights();

	void RenderShadowMaps();
	void UpdateGizmos();
	void RenderGizmos();
	void RenderDebugMode();

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

	// Editor state
	EditorSpecificData m_editor_data;
	EditorWindowSettings m_window_settings;
	EditorSettings m_editor_settings;

	// PBR Pipeline
	Pipeline* m_pbr_pipeline;
	DescriptorSet* m_pbr_global_descriptor;
	UniformBuffer* m_pbr_global_ubo;
	UniformBuffer* m_pbr_lights_ubo;
	UniformBuffer* m_global_illumination_ubo;
	UniformBuffer* m_shadow_ubo;

	GlobalData m_global_data;
	LightsUBOData m_lights_data;

	// Shadow Maps
	std::vector<VulkanTexture2D*> m_shadow_maps;
	DescriptorSet* m_textures_descriptor;
	Pipeline* m_shadow_pipeline;

	// Debug Visualization
	b8 m_show_gizmos;

	Buffer* m_gizmo_vb;
	Buffer* m_gizmo_ib;
	std::vector<VertexGuizmo> m_gizmo_vertices;
	std::vector<u32> m_gizmo_indices;
};
