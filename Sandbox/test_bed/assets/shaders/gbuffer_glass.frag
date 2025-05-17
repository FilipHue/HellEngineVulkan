#version 450

layout (set = 2, binding = 0) uniform sampler2D samplerTexture;

layout (set = 3, binding = 0) uniform MaterialUBO {
	vec4 baseColor;
} material;

layout (location = 1) in vec2 inUV;

layout (location = 0) out vec4 outColor;

layout (constant_id = 0) const float NEAR_PLANE = 0.1f;
layout (constant_id = 1) const float FAR_PLANE = 1000.0f;

float linearDepth(float depth)
{
	float z = depth * 2.0f - 1.0f; 
	return (2.0f * NEAR_PLANE * FAR_PLANE) / (FAR_PLANE + NEAR_PLANE - z * (FAR_PLANE - NEAR_PLANE));	
}

void main () 
{
	outColor = texture(samplerTexture, inUV);
}
