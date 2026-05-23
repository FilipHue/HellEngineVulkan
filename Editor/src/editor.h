#pragma once

// Internal
#include "hellengine/hellengine.h"

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

	void CreateResources();
	void CreatePipelines();
	void CreateDescriptors();

	void DestroyResources();

	// Editor
	void CreateEditorPipeline();
	void CreateEditorResources();
	void DestroyEditorResources();
	void CreateEditorDescriptors();
	void UpdateEditorDescriptors();
	void RenderEditor();

	void CreateEditorUI();

	// PBR
	void CreatePBRPipeline();
	void CreatePBRResources();
	void DestroyPBRResources();
	void CreatePBRDescriptors();
	void UpdatePBRDescriptors();
	void RenderPBR();

	// GBuffer
	void CreateGBufferPipeline();
	void CreateGBufferResources();
	void DestroyGBufferResources();
	void CreateGBufferDescriptors();
	void UpdateGBufferDescriptors();
	void RenderGBuffer();

	// Global Illumination
	void CreateGIPipeline();
	void CreateGIResources();
	void DestroyGIResources();
	void CreateGIDescriptors();
	void UpdateGIDescriptors();
	void RenderGI();

	// Shadow Maps
	void CreateShadowPipeline();
	void CreateShadowResources();
	void DestroyShadowResources();
	void CreateShadowDescriptors();
	void UpdateShadowDescriptors();
	void RenderShadowMaps();

	// Debug Visualization
	void CreateDebugPipeline();
	void CreateDebugResources();
	void DestroyDebugResources();
	void CreateDebugDescriptors();
	void UpdateDebugDescriptors();
	void UpdateDebug();
	void RenderDebug();

	void UpdateLightGizmos();

	// Final Blit
	void DrawToSwapchain();

	// Gizmos
	void ShowTransformGizmo();

	// Lighting
	void LoadLightData();
	glm::mat4 ComputeCascadeMatrix(const glm::vec3& lightDir, f32 cascadeNear, f32 cascadeFar, f32 shadowMapSize);
	void LoadShadowData(const glm::vec3& lightDir, u32 lightIndex, LightGPUData& gpuLight);

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
	EditorSettings m_editor_settings;

	// PBR Pipeline
	DescriptorSet* m_pbr_descriptor;
	DescriptorSet* m_pbr_gi_descriptor;
	UniformBuffer* m_pbr_global_data_ubo;
	UniformBuffer* m_pbr_lights_data_ubo;
	UniformBuffer* m_pbr_gi_data_ubo;
	UniformBuffer* m_pbr_shadow_data_ubo;

	// GBuffer Pipeline
	VulkanTexture2D* m_gbuffer_position = nullptr;
	VulkanTexture2D* m_gbuffer_normal = nullptr;
	VulkanTexture2D* m_gbuffer_albedo_ao = nullptr;
	VulkanTexture2D* m_gbuffer_depth = nullptr;
	VulkanTexture2D* m_gbuffer_lighting = nullptr;

	// Global Illumination
	VulkanTexture2D* m_gi_texture = nullptr;

	DescriptorSet* m_gi_descriptor;

	DynamicRenderingInfo m_gbuffer_rendering_info;
	DynamicRenderingInfo m_gi_rendering_info;

	GlobalData m_global_data;
	LightsUBOData m_lights_data;

	// Shadow Maps
	std::vector<VulkanTexture2D*> m_shadow_maps;
	DescriptorSet* m_textures_descriptor;

	Buffer* m_gizmo_vb;
	Buffer* m_gizmo_ib;
	std::vector<VertexGuizmo> m_gizmo_vertices;
	std::vector<u32> m_gizmo_indices;
};
