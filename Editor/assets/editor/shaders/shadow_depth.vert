#version 460
#extension GL_ARB_shader_draw_parameters : enable

// Per-vertex attributes
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inUV;
layout(location = 3) in vec3 inNormal;
layout(location = 4) in vec3 inTangent;
layout(location = 5) in vec3 inBitangent;

// Per-object data (must match CPU InstanceData struct and pbr.vert)
struct PerObjectData {
	mat4 model;
	uint material_index;
	int  entity_id;
};

layout(set = 2, binding = 0, std430) readonly buffer PerObjectSSBO
{
	PerObjectData obj[];
} perObject;

// Push constant: shadow view-projection matrix
layout(push_constant) uniform ShadowPushConstants {
	mat4 lightViewProj;
} pc;

void main()
{
	// Use gl_DrawIDARB to access per-draw data
	uint objectIndex = gl_DrawIDARB;
	mat4 model = perObject.obj[objectIndex].model;
	gl_Position = pc.lightViewProj * model * vec4(inPosition, 1.0);
}
