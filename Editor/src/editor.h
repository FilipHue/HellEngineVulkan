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

struct EditorSpecificData
{
	u32 show_debug_info = 0;
};

ALIGN_AS(64) struct GlobalShaderData
{
	glm::mat4 proj;
	glm::mat4 view;
	glm::vec3 camera_position;
	f32 _pad0;
	glm::vec4 ambient_color_intensity;  // xyz = color, w = intensity
};

// Global Illumination Settings
ALIGN_AS(64) struct GlobalIlluminationData
{
	// SSGI Settings
	u32 enabled;              // 0 = off, 1 = on
	u32 sample_count;         // Number of samples per pixel (4-32 typical)
	f32 ray_distance;         // Maximum ray march distance
	f32 intensity;            // GI contribution multiplier

	f32 thickness;            // Surface thickness for depth comparison
	f32 falloff;              // Distance falloff exponent
	f32 bias;                 // Depth bias to reduce self-occlusion
	f32 temporal_weight;      // Temporal accumulation weight (0-1)

	glm::vec4 debug_visualization;  // xyz = debug color, w = debug mode (0=off, 1=samples, 2=depth, etc.)

	f32 _pad0;
	f32 _pad1;
	f32 _pad2;
	f32 _pad3;
};

#define MAX_LIGHTS 16
#define MAX_SHADOW_CASCADES 4

ALIGN_AS(16) struct LightGPUData
{
	glm::vec4 position_type;     // xyz = position (or direction for directional), w = type (0=point, 1=directional, 2=spot)
	glm::vec4 color_intensity;   // xyz = color, w = intensity
	glm::vec4 direction_range;   // xyz = direction (for spot), w = range
	glm::vec4 cone_attenuation;  // x = inner cone, y = outer cone, z = attenuation, w = enabled (1.0 = on, 0.0 = off)
	glm::mat4 shadow_matrix;     // Shadow projection matrix for this light
	glm::vec4 shadow_params;     // x = shadow map index, y = shadow bias, z = shadow strength, w = cast_shadows (1.0 = yes)
};

// Shadow Settings
ALIGN_AS(64) struct ShadowSettings
{
	u32 enabled;                 // Global shadow toggle
	u32 shadow_map_size;         // Resolution per shadow map (e.g., 2048)
	f32 cascade_split_lambda;    // CSM split lambda (0.5-1.0)
	f32 min_bias;                // Minimum shadow bias

	f32 max_bias;                // Maximum shadow bias
	f32 normal_offset;           // Normal-based offset
	u32 pcf_samples;             // PCF filter samples (0=none, 1=3x3, 2=5x5)
	f32 softness;                // Shadow edge softness

	glm::vec4 cascade_distances; // Cascade split distances for directional lights

	f32 _pad0;
	f32 _pad1;
	f32 _pad2;
	f32 _pad3;
};

ALIGN_AS(16) struct LightsUBOData
{
	u32 light_count;
	u32 _pad0;
	u32 _pad1;
	u32 _pad2;
	LightGPUData lights[MAX_LIGHTS];
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

	void UpdateLights();  // Gather lights from scene and update UBO

	void RenderShadowMaps();    // Render shadow maps for shadow-casting lights
	void UpdateLightGizmos();   // Generate light gizmo geometry from scene
	void RenderLightGizmos();   // Draw light gizmos
	void RenderDebugMode();     // Render with active debug visualization mode

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
	EditorSettings* m_editor_settings;

	// PBR Pipeline
	Pipeline* m_pbr_pipeline;
	DescriptorSet* m_pbr_global_descriptor;
	UniformBuffer* m_pbr_global_ubo;
	UniformBuffer* m_pbr_lights_ubo;
	UniformBuffer* m_gi_settings_ubo;  // Global Illumination settings
	UniformBuffer* m_shadow_settings_ubo;  // Shadow settings

	GlobalShaderData m_global_shader_data;
	LightsUBOData m_lights_ubo_data;
	GlobalIlluminationData m_gi_data;  // GI settings
	ShadowSettings m_shadow_settings;  // Shadow settings

	// Shadow Maps
	std::vector<VulkanTexture2D*> m_shadow_maps;  // Shadow map textures for each light
	DescriptorSet* m_textures_descriptor;  // Combined texture descriptor (set 3: shadow maps + material textures)
	Pipeline* m_shadow_pipeline;  // Shadow depth-only rendering pipeline

	// Debug Visualization
	b8 m_show_light_gizmos;

	Buffer* m_light_gizmo_vb;
	Buffer* m_light_gizmo_ib;
	std::vector<VertexGuizmo> m_light_gizmo_vertices;
	std::vector<u32> m_light_gizmo_indices;
};

