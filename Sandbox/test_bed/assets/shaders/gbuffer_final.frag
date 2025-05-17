#version 450

layout(set = 0, binding = 0) uniform sampler2D colorGlassSample;
layout(set = 0, binding = 1) uniform sampler2D depthGlassSample;
layout(set = 0, binding = 2) uniform sampler2D colorCompositionSample;
layout(set = 0, binding = 3) uniform sampler2D depthCompositionSample;

layout(location = 0) in vec2 inUV;
layout(location = 0) out vec4 outColor;

void main()
{
    vec4 baseColor = texture(colorCompositionSample, inUV);
    vec4 glassColor = texture(colorGlassSample, inUV);
    float depthGlass = texture(depthGlassSample, inUV).r;
    float depthBase = texture(depthCompositionSample, inUV).r;

    // Guard: if no glass rendered here (alpha = 0 or invalid depth), return base
    if (glassColor.a <= 0.01 || depthGlass >= 1.0) {
        outColor = baseColor;
        return;
    }

    // If glass is in front of opaque scene, blend it
    if (depthGlass < depthBase) {
        vec3 blended = mix(baseColor.rgb, glassColor.rgb, glassColor.a);
        outColor = vec4(blended, 1.0);
    } else {
        outColor = baseColor;
    }

    //outColor = baseColor;
}
