#pragma once

#include "hellengine/hellengine.h"
#include "shared.h"
#include "panels/editor_hierarchy.h"
#include "panels/editor_inspector.h"

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

struct GridCameraData
{
	glm::mat4 proj;
	glm::mat4 view;
	glm::vec3 pos;
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
	void CreateAttachments();
	void CreateDescriptors();

	void CreateEditorPanels();

	void OnViewportResize();

	void DrawGrid();
	void DrawToSwapchain();

	void MenuBar();

	void ShowGuizmo();

	// TEMP
	void LoadResourcesForTest();

private:
	// Editor
	MultiProjectionCamera m_editor_camera;
	MultiProjectionController m_editor_camera_controller;

	Pipeline m_editor_pipeline;

	// Viewport
	glm::vec4 m_viewport_clear_color;
	glm::vec2 m_viewport_clear_depth;
	glm::uvec2 m_viewport_size;

	DescriptorSet m_viewport_descriptor;

	Texture2D m_viewport_color_texture;
	Texture2D m_viewport_pick_texture;
	Texture2D m_viewport_depth_texture;

	DynamicRenderingAttachmentInfo m_viewport_color_attachment;
	DynamicRenderingAttachmentInfo m_viewport_pick_attachment;
	DynamicRenderingAttachmentInfo m_viewport_depth_attachment;
	DynamicRenderingInfo m_viewport_dri;

	Pipeline m_grid_pipeline;
	DescriptorSet m_grid_descriptor;
	UniformBuffer m_grid_buffer;

	Texture2D m_grid_color_texture;
	Texture2D m_grid_depth_texture;

	DynamicRenderingAttachmentInfo m_grid_color_attachment;
	DynamicRenderingAttachmentInfo m_grid_depth_attachment;
	DynamicRenderingInfo m_grid_dri;

	GridCameraData m_grid_data;

	// Editor UI
	EditorHierarchy* m_hierarchy_panel;
	EditorInspector* m_inspector_panel;

	Viewport* m_viewport_panel;
	Bounds2D m_viewport_panel_bounds;
	glm::uvec2 m_viewport_panel_size;

	// PBR Pipeline
	GlobalShaderData m_global_shader_data;

	// TEMP
	Pipeline m_test_pipeline;
	DescriptorSet m_test_descriptor;
	UniformBuffer m_test_ubo;
};

