#pragma once

#include "hellengine/hellengine.h"

#include "../shared.h"
#include "editor_hierarchy.h"

using namespace hellengine;
using namespace core;
using namespace ecs;
using namespace graphics;
using namespace ui;
using namespace math;
using namespace resources;
using namespace tools;

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

	void SetViewportEditorReferences(MultiProjectionCamera* camera);

	void CreateViewportResources();

	void UpdateGridCameraData();
	void OnViewportResize();
	void OnMouseButtonPressed();

	Texture2D* GetColorTexture() const { return m_viewport_color_texture; }
	Texture2D* GetDepthTexture() const { return m_viewport_depth_texture; }
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
	DescriptorSet* m_viewport_descriptor;

	Texture2D* m_viewport_color_texture;
	Texture2D* m_viewport_pick_texture;
	Texture2D* m_viewport_depth_texture;

	DynamicRenderingAttachmentInfo m_viewport_color_attachment;
	DynamicRenderingAttachmentInfo m_viewport_pick_attachment;
	DynamicRenderingAttachmentInfo m_viewport_depth_attachment;
	DynamicRenderingInfo m_viewport_dri;

	b8 m_can_pick;

	// Viewport grid
	UniformBuffer* m_grid_buffer;
	DescriptorSet* m_grid_descriptor;

	Texture2D* m_grid_color_texture;
	Texture2D* m_grid_depth_texture;

	DynamicRenderingAttachmentInfo m_grid_color_attachment;
	DynamicRenderingAttachmentInfo m_grid_depth_attachment;
	DynamicRenderingInfo m_grid_dri;

	GridCameraData m_grid_data;

	// Editor references
	MultiProjectionCamera* m_editor_camera;
	EditorHierarchy* m_hierarchy_panel;

	VulkanBackend* m_backend;
	VulkanFrontend* m_frontend;
};
