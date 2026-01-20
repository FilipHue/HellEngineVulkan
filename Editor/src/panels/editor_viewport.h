#pragma once

#include "hellengine/hellengine.h"

#include "editor_hierarchy.h"

using namespace hellengine;
using namespace core;
using namespace ecs;
using namespace graphics;
using namespace ui;
using namespace math;
using namespace resources;
using namespace tools;

constexpr auto EDITOR_SHADER_PATH = "assets/editor/shaders";
constexpr auto EDITOR_MODEL_PATH = "assets/models";

constexpr auto VIEWPORT_COLOR = "EDITOR_VIEWPORT_COLOR_ATTACHMENT";
constexpr auto VIEWPORT_PICK = "EDITOR_VIEWPORT_PICK_ATTACHMENT";
constexpr auto VIEWPORT_DEPTH = "EDITOR_VIEWPORT_DEPTH_ATTACHMENT";

struct GridCameraData
{
	glm::mat4 proj;
	glm::mat4 view;
	glm::vec3 pos;
};

class EditorViewport : public Viewport
{
public:
	EditorViewport();
	virtual ~EditorViewport();

	void Init(VulkanBackend* backend, VulkanFrontend* frontend, EditorHierarchy* hierarchy_panel);

	void RenderBegin();
	void RenderUpdate();
	void RenderEnd();

	void SetViewportClearColor(const glm::vec4& color);
	void SetViewportClearDepth(const glm::vec2& depth);

	void CanPick(b8 can_pick) { m_can_pick = can_pick; }

	void SetViewportEditorReferences(Pipeline pipeline, MultiProjectionCamera* camera);

	void CreateViewportResources();

	void UpdateGridCameraData();
	void OnViewportResize();
	void OnMouseButtonPressed();
private:
	void CreatePipelines();
	void CreateAttachments();
	void CreateDescriptors();

	void DrawGrid();

private:
	// Clear values
	glm::vec4 m_clear_color;
	glm::vec2 m_depth_color;

	// Viewport
	DescriptorSet m_viewport_descriptor;

	Texture2D m_viewport_color_texture;
	Texture2D m_viewport_pick_texture;
	Texture2D m_viewport_depth_texture;

	DynamicRenderingAttachmentInfo m_viewport_color_attachment;
	DynamicRenderingAttachmentInfo m_viewport_pick_attachment;
	DynamicRenderingAttachmentInfo m_viewport_depth_attachment;
	DynamicRenderingInfo m_viewport_dri;

	b8 m_can_pick;

	// Viewport grid
	Pipeline m_grid_pipeline;
	UniformBuffer m_grid_buffer;
	DescriptorSet m_grid_descriptor;

	Texture2D m_grid_color_texture;
	Texture2D m_grid_depth_texture;

	DynamicRenderingAttachmentInfo m_grid_color_attachment;
	DynamicRenderingAttachmentInfo m_grid_depth_attachment;
	DynamicRenderingInfo m_grid_dri;

	GridCameraData m_grid_data;

	// Editor references
	MultiProjectionCamera* m_editor_camera;
	Pipeline m_editor_pipeline;
	EditorHierarchy* m_hierarchy_panel;

	VulkanBackend* m_backend;
	VulkanFrontend* m_frontend;
};
