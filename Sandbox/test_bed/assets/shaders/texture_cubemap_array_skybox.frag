#version 450

layout (set = 0, binding = 1) uniform samplerCubeArray samplerCubeMapArray;

layout (binding = 0) uniform UBO
{
	mat4 projection;
	mat4 model;
	mat4 invModel;
	float lodBias;
	int cubeMapIndex;
} ubo;

layout (location = 0) in vec3 inUV;

layout (location = 0) out vec4 outFragColor;

void main() 
{
	outFragColor = textureLod(samplerCubeMapArray, vec4(inUV, ubo.cubeMapIndex), ubo.lodBias);
}