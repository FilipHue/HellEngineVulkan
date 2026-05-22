#version 460

layout(location = 0) in vec2 vUV;
layout(location = 0) out vec4 oColor;

layout(set = 0, binding = 0) uniform sampler2D uSceneColor;
layout(set = 0, binding = 1) uniform sampler2D uSSGIRaw;

layout(push_constant) uniform PC
{
    float strength;
} pc;

void main()
{
    vec3 scene = texture(uSceneColor, vUV).rgb;
    vec3 gi    = texture(uSSGIRaw,    vUV).rgb;

    // Basic energy control
    vec3 outColor = scene + gi * pc.strength;

    // Optional gentle clamp for stability
    outColor = clamp(outColor, vec3(0.0), vec3(20.0));

    oColor = vec4(outColor, 1.0);
}
