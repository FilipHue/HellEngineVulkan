#version 450

layout(set = 2, binding = 0) uniform sampler2D texSampler1;
layout(set = 2, binding = 1) uniform sampler2D texSampler2;

layout (location = 0) in vec3 inColor;
layout (location = 1) in vec2 inUV;

layout (location = 0) out vec4 outFragColor;

void main() 
{
   vec4 color1 = texture(texSampler1, inUV);
   vec4 color2 = texture(texSampler2, inUV);

   outFragColor = mix(color1, color2, 0.5f);
}