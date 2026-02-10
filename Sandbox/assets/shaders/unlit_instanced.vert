// assets/shaders/unlit_instanced.vert
#version 460

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec4 aColor;
layout(location = 2) in vec2 aUV;
layout(location = 3) in vec3 aNormal;

layout(location = 0) out vec4 vColor;

layout(set = 0, binding = 0) uniform CameraUBO
{
	mat4 view;
	mat4 projection;
} uCamera;

struct InstanceData
{
	vec4 tint;
	mat4 model;
};

layout(std430, set = 1, binding = 0) readonly buffer InstanceSSBO
{
	InstanceData data[];
} instances;

void main()
{
	mat4 M = instances.data[gl_InstanceIndex].model;
	vec4 tint = instances.data[gl_InstanceIndex].tint;
	gl_Position = uCamera.projection * uCamera.view * M * vec4(aPos, 1.0);
	vColor = aColor * tint;
}
