#version 450

layout (set = 2, binding = 0) uniform materialUBO {
	vec4 baseColor;
} material;
layout (set = 2, binding = 1) uniform sampler2D samplerColorMap;

layout (location = 0) in vec3 inColor;
layout (location = 1) in vec2 inUV;

layout (location = 0) out vec4 outFragColor;

void main() 
{
	vec4 color = texture(samplerColorMap, inUV) * vec4(inColor, 1.0);

	outFragColor = color;
}