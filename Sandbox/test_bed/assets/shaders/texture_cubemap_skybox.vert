#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inUV;
layout(location = 3) in vec3 inNormal;

layout (location = 0) out vec3 outUV;

layout (set = 0, binding = 0) uniform UBO 
{
	mat4 projection;
	mat4 model;
} ubo;

void main() 
{
	outUV = inPosition;

	mat4 viewMat = mat4(mat3(ubo.model));
	gl_Position = ubo.projection * viewMat * vec4(inPosition.xyz, 1.0);
}