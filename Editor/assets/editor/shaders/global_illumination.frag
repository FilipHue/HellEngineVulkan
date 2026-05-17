#version 460

#include "includes/common.glsl"
#include "includes/global_illumination.glsl"

layout(location = 0) in vec2 vUV;
layout(location = 0) out vec4 outGI;

layout(set = 0, binding = 0) uniform GlobalData
{
    CameraData camera;
    WorldData world;
} ubo_globalData;

layout(set = 0, binding = 1) uniform GlobalIlluminationData
{
    GlobalIlluminationSettings settings;
} ubo_giData;

layout(set = 0, binding = 2) uniform sampler2D gPosition;
layout(set = 0, binding = 3) uniform sampler2D gNormal;
layout(set = 0, binding = 4) uniform sampler2D gAlbedoAO;
layout(set = 0, binding = 5) uniform sampler2D gDepth;

vec2 WorldToScreenUV(vec3 worldPos)
{
    vec4 clip = ubo_globalData.camera.projection * ubo_globalData.camera.view * vec4(worldPos, 1.0);

    if (clip.w <= 0.0)
    {
        return vec2(-1.0);
    }

    vec3 ndc = clip.xyz / clip.w;

    return ndc.xy * 0.5 + 0.5;
}

bool IsInsideScreen(vec2 uv)
{
    return uv.x >= 0.0 &&
           uv.x <= 1.0 &&
           uv.y >= 0.0 &&
           uv.y <= 1.0;
}

vec3 ComputeGlobalIllumination(vec2 uv)
{
    if (ubo_giData.settings.enabled == 0u)
    {
        return vec3(0.0);
    }

    vec3 worldPos = texture(gPosition, uv).xyz;
    vec3 normal = normalize(texture(gNormal, uv).xyz * 2.0 - 1.0);
    vec4 albedoAO = texture(gAlbedoAO, uv);

    vec3 albedo = albedoAO.rgb;
    float ao = albedoAO.a;

    uint sampleCount = max(ubo_giData.settings.sample_count, 1u);

    vec3 ambientLight =
        ubo_globalData.world.ambient_color_intensity.xyz *
        ubo_globalData.world.ambient_color_intensity.w;

    vec2 resolution = vec2(textureSize(gPosition, 0));

    vec3 gi = vec3(0.0);
    float validSamples = 0.0;

    for (uint i = 0u; i < sampleCount; ++i)
    {
        vec2 seed = uv * resolution + vec2(float(i), float(i * 17u));
        vec3 rayDir = RandomHemisphereDirection(normal, seed);

        float sampleT = (float(i) + 0.5) / float(sampleCount);
        float sampleDistance = sampleT * ubo_giData.settings.ray_distance;

        vec3 sampleWorldPos = worldPos + rayDir * sampleDistance;
        vec2 sampleUV = WorldToScreenUV(sampleWorldPos);

        if (!IsInsideScreen(sampleUV))
        {
            continue;
        }

        vec3 sceneWorldPos = texture(gPosition, sampleUV).xyz;
        vec3 sceneNormal = normalize(texture(gNormal, sampleUV).xyz * 2.0 - 1.0);
        vec4 sceneAlbedoAO = texture(gAlbedoAO, sampleUV);

        float hitDistance = length(sceneWorldPos - worldPos);
        float rayDistanceError = abs(hitDistance - sampleDistance);

        if (rayDistanceError > ubo_giData.settings.thickness)
        {
            continue;
        }

        float nDotRay = max(dot(normal, rayDir), 0.0);
        float hitFacing = max(dot(sceneNormal, -rayDir), 0.0);

        float distanceFalloff =
            1.0 / (1.0 + pow(hitDistance, ubo_giData.settings.falloff));

        vec3 bouncedLight = ambientLight * sceneAlbedoAO.rgb;

        vec3 contribution =
        bouncedLight *
        nDotRay *
        hitFacing *
        distanceFalloff;

        contribution = min(contribution, vec3(0.25));

        gi += contribution;

        validSamples += 1.0;
    }

    if (validSamples > 0.0)
    {
        gi /= validSamples;
    }
    else
    {
        gi = ambientLight;
    }

    gi *= albedo;
    gi *= ao;
    gi *= ubo_giData.settings.intensity;

    return gi;
}

void main()
{
    vec3 gi = ComputeGlobalIllumination(vUV);
    gi = max(gi, vec3(0.0));
    gi = min(gi, vec3(1.0));
    outGI = vec4(gi, 1.0);
}