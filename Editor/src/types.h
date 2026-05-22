#pragma once

//Internal
#include <hellengine/core/core.h>
#include <hellengine/graphics/graphics_core.h>
#include <hellengine/math/core.h>

using namespace hellengine::math;

ALIGN_AS(LAYOUT_STD140_ALIGNMENT) struct WorldData
{
	glm::vec4 time;						// x = total time, y = delta time
};
 
ALIGN_AS(LAYOUT_STD140_ALIGNMENT) struct GlobalData
{
	CameraData camera;
	WorldData world;
};

#define MAX_LIGHTS 16
#define MAX_SHADOW_CASCADES 4

ALIGN_AS(LAYOUT_STD140_ALIGNMENT) struct LightGPUData
{
	glm::vec4 position_type;
	glm::vec4 direction_range;
	glm::vec4 color_intensity;
	glm::vec4 cone_attenuation;
	glm::vec4 shadow_params;

	glm::mat4 shadow_matrix;

	glm::mat4 cascade_matrices[MAX_SHADOW_CASCADES];
	glm::vec4 cascade_splits;
};

ALIGN_AS(LAYOUT_STD140_ALIGNMENT) struct LightsUBOData
{
	u32 light_count;
	u32 _pad0;
	u32 _pad1;
	u32 _pad2;
	LightGPUData lights[MAX_LIGHTS];
};


