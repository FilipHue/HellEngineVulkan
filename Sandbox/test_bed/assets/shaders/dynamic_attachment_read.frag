#version 450

layout(set = 0, binding = 0) uniform sampler2D colorSample;
layout(set = 0, binding = 1) uniform sampler2D depthSample;
layout(set = 1, binding = 0) uniform DataUbo {
    float brightness;
    float contrast;
    vec2 range;
} data;

layout(location = 0) in vec2 v_UV;
layout(location = 0) out vec4 outColor;

layout(push_constant) uniform PushConsts {
    int textureIndex;
} push_consts;

vec3 brightnessContrast(vec3 color, float brightness, float contrast) {
	return (color - 0.5) * contrast + 0.5 + brightness;
}

void main()
{
    if (push_consts.textureIndex == 0) {
        vec3 color = texture(colorSample, v_UV).rgb;
        outColor = vec4(brightnessContrast(color, data.brightness, data.contrast), 1.0);
    } else if (push_consts.textureIndex == 1) {
        float depth = texture(depthSample, v_UV).r;
        outColor = vec4(vec3((depth - data.range[0]) * 1.0 / (data.range[1] - data.range[0])), 1.0);
    }
}
