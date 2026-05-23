// ====================================
// Global Illumination Fragment Shader
// ====================================
#version 460

// Includes
#include "includes/global_illumination.glsl"

// Inputs
layout(location = 0) in vec2 vUV;

// Outputs
layout(location = 0) out vec4 outGI;

// Descriptors
layout(set = 0, binding = 0) uniform GlobalData
{
    CameraData camera;
    WorldData world;
} ubo_globalData;

layout(set = 0, binding = 1) uniform GlobalIlluminationData
{
    GlobalIlluminationSettings settings;
} ubo_giData;

// Uniforms
layout(set = 0, binding = 2) uniform sampler2D gPosition;
layout(set = 0, binding = 3) uniform sampler2D gNormal;
layout(set = 0, binding = 4) uniform sampler2D gAlbedoAO;
layout(set = 0, binding = 5) uniform sampler2D gDepth;
layout(set = 0, binding = 6) uniform sampler2D gLighting;

vec3 ComputeGlobalIllumination(vec2 uv, GlobalIlluminationSettings settings, mat4 projection, mat4 view)
{
    if (settings.enabled == 0u)
    {
        return vec3(0.0);
    }

    // Skip background pixels (no geometry)
    float depth = texture(gDepth, uv).r;
    if (depth >= 1.0) return vec3(0.0);

    // Skip fully shadowed surfaces
    vec3 directLight = texture(gLighting, uv).rgb;
    if (dot(directLight, vec3(0.333)) < 0.001) return vec3(0.0);

    vec3 worldPos = texture(gPosition, uv).xyz;
    vec3 normal = normalize(texture(gNormal, uv).xyz * 2.0 - 1.0);
    vec4 albedoAO = texture(gAlbedoAO, uv);

    vec3 albedo = albedoAO.rgb;
    float ao = albedoAO.a;

    uint sampleCount = max(settings.sample_count, 1u);

    vec2 resolution = vec2(textureSize(gPosition, 0));

    vec3 gi = vec3(0.0);
    float validSamples = 0.0;

    for (uint i = 0u; i < sampleCount; ++i)
    {
        vec2 seed = uv * resolution + vec2(float(i), float(i * 17u));
        vec3 rayDir = RandomHemisphereDirection(normal, seed);

        float sampleT = (float(i) + 0.5) / float(sampleCount);
        float sampleDistance = sampleT * settings.ray_distance;

        vec3 sampleWorldPos = worldPos + rayDir * sampleDistance;
        vec2 sampleUV = WorldToScreenUV(sampleWorldPos, projection, view);

        if (!IsInsideScreen(sampleUV))
        {
            continue;
        }

        vec3 sceneWorldPos = texture(gPosition, sampleUV).xyz;
        vec3 sceneNormal = normalize(texture(gNormal, sampleUV).xyz * 2.0 - 1.0);
        vec4 sceneAlbedoAO = texture(gAlbedoAO, sampleUV);

        float hitDistance = length(sceneWorldPos - worldPos);
        float rayDistanceError = abs(hitDistance - sampleDistance);

        if (rayDistanceError > settings.thickness)
        {
            continue;
        }

        float nDotRay = max(dot(normal, rayDir), 0.0);
        float hitFacing = max(dot(sceneNormal, -rayDir), 0.0);
        if (hitFacing < 0.1) continue;

        float distanceFalloff = 1.0 / (1.0 + pow(hitDistance, settings.falloff));

        vec3 sceneLighting = texture(gLighting, sampleUV).rgb;
        vec3 bouncedLight = sceneLighting * sceneAlbedoAO.rgb;

        vec3 contribution =
            bouncedLight *
            nDotRay *
            hitFacing *
            distanceFalloff;

        float lum = dot(contribution, vec3(0.2126, 0.7152, 0.0722));
        if (lum > 0.25) contribution *= 0.25 / lum;

        gi += contribution;
        validSamples += 1.0;
    }

    if (validSamples > 0.0)
    {
        gi /= validSamples;
    }
    else
    {
        gi = vec3(0.0);
    }

    gi *= albedo;
    gi *= ao;
    gi *= settings.intensity;

    return gi;
}

void main()
{
    vec3 gi = ComputeGlobalIllumination(vUV, ubo_giData.settings, ubo_globalData.camera.projection, ubo_globalData.camera.view);
    if (ubo_giData.settings.enabled == 1u)
    {
        outGI = vec4(gi, 1.0);
    }
    else
    {
        outGI = vec4(0.0);
    }
}
