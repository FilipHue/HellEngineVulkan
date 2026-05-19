#pragma once

// Internal
#include <hellengine/hellengine.h>

#include "shared.h"
#include "types.h"

// External
#include <yaml-cpp/yaml.h>

using namespace hellengine;
using namespace core;
using namespace ecs;
using namespace graphics;
using namespace ui;
using namespace math;
using namespace resources;
using namespace tools;

enum EditorRenderMode
{
	EditorRenderMode_Normal = 0,
	EditorRenderMode_Wireframe = 1,
	EditorRenderMode_UVs = 2,
	EditorRenderMode_Normals = 3,
	EditorRenderMode_ShadowMap = 4,

	EditorRenderMode_Count
};

// Global Illumination Settings
ALIGN_AS(LAYOUT_STD140_ALIGNMENT) struct GlobalIlluminationSettings
{
	u32 enabled;              // 0 = off, 1 = on
	u32 sample_count;         // Number of samples per pixel (4-32 typical)
	f32 ray_distance;         // Maximum ray march distance
	f32 intensity;            // GI contribution multiplier

	f32 thickness;            // Surface thickness for depth comparison
	f32 falloff;              // Distance falloff exponent
	f32 bias;                 // Depth bias to reduce self-occlusion
	f32 temporal_weight;      // Temporal accumulation weight (0-1)

	glm::vec4 debug_visualization;  // xyz = debug color, w = debug mode (0=off, 1=samples, 2=depth, etc.)
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

struct EditorSettings
{
public:
	EditorSettings() = default;
	~EditorSettings() = default;

	void Serialize();
	void Deserialize();

private:
	void LoadDefaults();

	void SerializeGISettings(YAML::Node& root);
	void SerializeShadowSettings(YAML::Node& root);
	void SerializeWindowSettings(YAML::Node& root);

	void DeserializeGISettings(const YAML::Node& root);
	void DeserializeShadowSettings(const YAML::Node& root);
	void DeserializeWindowSettings(const YAML::Node& root);

public:
	b8 show_gizmos = true;
	b8 show_debug = false;
	b8 show_grid = true;

	EditorRenderMode render_mode = EditorRenderMode_Normal;
	GlobalIlluminationSettings gi_settings = {};
	ShadowSettings shadow_settings = {};
	EditorWindowSettings window_settings = {};

private:

	std::string m_config_file = CONCAT_PATHS(EDITOR_ASSET_PATH, "editor_settings.yaml");
};
