#version 450

layout(set = 0, binding = 0) uniform sampler2D colorSample;

layout(location = 0) in vec2 v_UV;
layout(location = 0) out vec4 outColor;

void main()
{
    outColor = vec4(texture(colorSample, v_UV).rgb, 1.0);
}
