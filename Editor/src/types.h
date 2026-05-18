#pragma once

//Internal
#include <hellengine/core/core.h>
#include <hellengine/graphics/graphics_core.h>
#include <hellengine/math/core.h>

using namespace hellengine::math;

ALIGN_AS(LAYOUT_STD140_ALIGNMENT) struct WorldData
{
	glm::vec4 time;						// x = total time, y = delta time
	glm::vec4 ambient_color_intensity;  // xyz = color, w = intensity
};
 
ALIGN_AS(LAYOUT_STD140_ALIGNMENT) struct GlobalData
{
	CameraData camera;
	WorldData world;
};

// Global Illumination Settings
ALIGN_AS(LAYOUT_STD140_ALIGNMENT) struct GlobalIlluminationSettings
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

ALIGN_AS(LAYOUT_STD140_ALIGNMENT) struct LightGPUData
{
	glm::vec4 position_type;     // xyz = position (or direction for directional), w = type (0=point, 1=directional, 2=spot)
	glm::vec4 color_intensity;   // xyz = color, w = intensity
	glm::vec4 direction_range;   // xyz = direction (for spot), w = range
	glm::vec4 cone_attenuation;  // x = inner cone, y = outer cone, z = attenuation, w = enabled (1.0 = on, 0.0 = off)
	glm::mat4 shadow_matrix;     // Shadow projection matrix for this light
	glm::vec4 shadow_params;     // x = shadow map index, y = shadow bias, z = shadow strength, w = cast_shadows (1.0 = yes)
};

// Shadow Settings
ALIGN_AS(LAYOUT_STD140_ALIGNMENT) struct ShadowSettings
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

ALIGN_AS(LAYOUT_STD140_ALIGNMENT) struct LightsUBOData
{
	u32 light_count;
	u32 _pad0;
	u32 _pad1;
	u32 _pad2;
	LightGPUData lights[MAX_LIGHTS];
};

enum EditorRenderMode
{
	EditorRenderMode_Normal = 0,
	EditorRenderMode_Wireframe = 1,
	EditorRenderMode_UVs = 2,
	EditorRenderMode_Normals = 3,
	EditorRenderMode_ShadowMap = 4,

	EditorRenderMode_Count
};

struct EditorSettings
{
	b8 show_gizmos = true;
	b8 show_grid = true;

	EditorRenderMode render_mode = EditorRenderMode_Normal;
	GlobalIlluminationSettings gi_settings = {};
	ShadowSettings shadow_settings = {};
};

struct EditorWindowSettings
{
	b8 show_hierarchy = true;
	b8 show_inspector = true;
	b8 show_editor_settings = false;
	b8 show_render_settings = false;
	b8 show_gi_settings = false;
	b8 show_shadow_settings = false;
	b8 show_about_window = false;
};
