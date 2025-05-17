#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

layout(set = 2, binding = 0) uniform sampler2D texSampler;

layout(set = 3, binding = 0) uniform ShaderData
{
    vec4 color;
} shaderData;

void main() {
   vec4 text_sample = texture(texSampler, fragTexCoord);

   outColor = shaderData.color * text_sample;
}