#pragma once

#include "hellengine/hellengine.h"

using namespace hellengine;
using namespace core;
using namespace graphics;
using namespace ui;
using namespace math;
using namespace resources;
using namespace tools;

constexpr auto EDITOR_SHADER_PATH = "assets/editor/shaders";
constexpr auto EDITOR_MODEL_PATH = "assets/models";

constexpr auto VIEWPORT_COLOR = "EDITOR_VIEWPORT_COLOR_ATTACHMENT";
constexpr auto VIEWPORT_DEPTH = "EDITOR_VIEWPORT_DEPTH_ATTACHMENT";

ALIGN_AS(64) struct GlobalShaderData
{
	glm::mat4 proj;
	glm::mat4 view;
};

ALIGN_AS(64) struct GridCameraData
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
	//Editor
	void CreateEditorPipeline();
	void CreateEditorResources();

	void DrawGrid();
	void DrawToSwapchain();

	// Viewport
	void InitializeViewportState();
	void CreateViewportAttachments();
	void CreateViewportResources();
	void ViewportResize();

	// TEMP
	void LoadResourcesForTest();

private:
	// Editor
	MultiProjectionCamera m_editor_camera;
	MultiProjectionController m_editor_camera_controller;

	Pipeline m_editor_pipeline;

	Pipeline m_grid_pipeline;
	DescriptorSet m_grid_descriptor;
	UniformBuffer m_grid_buffer;

	Texture2D m_grid_color_texture;
	Texture2D m_grid_depth_texture;

	DynamicRenderingAttachmentInfo m_grid_color_attachment;
	DynamicRenderingAttachmentInfo m_grid_depth_attachment;
	DynamicRenderingInfo m_grid_dri;

	GridCameraData m_grid_data;

	// Viewport
	glm::vec4 m_viewport_clear_color;
	glm::vec2 m_viewport_clear_depth;
	glm::uvec2 m_viewport_size;

	Pipeline m_viewport_pipeline;
	DescriptorSet m_viewport_descriptor;

	Texture2D m_viewport_color_texture;
	Texture2D m_viewport_depth_texture;

	DynamicRenderingAttachmentInfo m_viewport_color_attachment;
	DynamicRenderingAttachmentInfo m_viewport_depth_attachment;
	DynamicRenderingInfo m_viewport_dri;

	ViewportPanel* m_viewport_panel;

	// PBR Pipeline
	GlobalShaderData m_global_shader_data;

	Pipeline m_pbr_pipeline;

	// TEMP
	Model* m_sponza;
	Pipeline m_test_pipeline;
	DescriptorSet m_test_descriptor;
	UniformBuffer m_test_ubo;
};

