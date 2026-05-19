#version 460

#extension GL_EXT_nonuniform_qualifier : enable

#include "includes/common.glsl"
#include "includes/global_illumination.glsl"
#include "includes/pbr_common.glsl"
#include "includes/light.glsl"
#include "includes/shadow.glsl"

// Per-vertex inputs from vertex shader
layout(location = 0) in vec3 vColor;
layout(location = 1) in vec2 vUV;
layout(location = 2) in vec3 vPosWS;
layout(location = 3) in vec3 vNormalWS;
layout(location = 4) in vec3 vTangentWS;
layout(location = 5) in vec3 vBitangentWS;

// Per-object index to fetch data from SSBOs
layout(location = 6) in flat int vObjectIndex;

// Outputs
layout(location = 0) out vec4 outFragColor;
layout(location = 1) out uint outPickId;

// Global data
layout(set = 0, binding = 0) uniform GlobalData {
    CameraData camera;
    WorldData world;
} ubo_globalData;

// Light data
layout(set = 0, binding = 1) uniform LightData {
    uint light_count;
    uint _pad0;
    uint _pad1;
    uint _pad2;
    LightInfo lights[MAX_LIGHTS];
} ubo_lightData;

// Global Illumination data
layout(set = 0, binding = 2) uniform GlobalIlluminationData {
    GlobalIlluminationSettings settings;
    vec4 debug;  // xyz = debug color, w = debug mode
} ubo_giData;

// Shadows data
layout(set = 0, binding = 3) uniform ShadowData {
    ShadowSettings settings;
} ubo_shadowData;

// Global illumination data
layout(set = 0, binding = 4) uniform sampler2D globalIlluminationTexture;

// Material data
layout(set = 1, binding = 0, std430) readonly buffer MaterialSSBO {
    MaterialInfo data[];
} ssbo_materials;

// Per-object data
layout(set = 2, binding = 0, std430) readonly buffer PerObjectSSBO {
    PerObjectData data[];
} ssbo_objects;

// Texture samplers
layout(set = 3, binding = 0) uniform sampler2D shadowMaps[MAX_SHADOW_MAPS];
layout(set = 3, binding = 1) uniform sampler2D textures[];

layout(push_constant) uniform PushConstants {
    uint debug_view_mode;
} pc;

// ================================
// Functions
// ================================

vec4 SampleTexture(int index, vec2 uv)
{
    return texture(textures[nonuniformEXT(uint(index))], uv);
}

float SampleShadowMapPCF(uint shadowMapIndex, vec2 uv, float currentDepth, float bias)
{
    vec2 texelSize = 1.0 / vec2(textureSize(shadowMaps[nonuniformEXT(shadowMapIndex)], 0));
    int radius = int(ubo_shadowData.settings.pcf_samples);

    float result = 0.0;
    int count = 0;

    for (int x = -radius; x <= radius; ++x)
    {
        for (int y = -radius; y <= radius; ++y)
        {
            vec2 offset = vec2(x, y) * texelSize * ubo_shadowData.settings.softness;
            float closestDepth = texture(shadowMaps[nonuniformEXT(shadowMapIndex)], uv + offset).r;
            result += (currentDepth - bias) > closestDepth ? 0.0 : 1.0;
            count++;
        }
    }

    return result / float(count);
}

float CalculateShadow(LightInfo light, vec3 worldPos, vec3 normal, uint lightIndex)
{
    // Early out: shadows disabled globally or for this light
    if (ubo_shadowData.settings.enabled == 0u || light.shadow_params.w < 0.5)
        return 1.0;

    float lightType = light.position_type.w;
    vec3 N = normalize(normal);

    if (lightType != 1.0)
    {
        vec4 lightSpacePos = light.shadow_matrix * vec4(worldPos, 1.0);
        vec3 projCoords = lightSpacePos.xyz / lightSpacePos.w;
        projCoords.xy = projCoords.xy * 0.5 + 0.5;

        if (projCoords.x < 0.0 || projCoords.x > 1.0 ||
            projCoords.y < 0.0 || projCoords.y > 1.0 ||
            projCoords.z < 0.0 || projCoords.z > 1.0)
            return 1.0;

        vec3 lightDir = (lightType == 2.0)
            ? normalize(light.direction_range.xyz)
            : normalize(light.position_type.xyz - worldPos);

        float bias = max(
            ubo_shadowData.settings.max_bias * (1.0 - dot(N, lightDir)),
            ubo_shadowData.settings.min_bias
        );

        uint shadowMapIndex = uint(light.shadow_params.x); //  use this, not lightIndex
        return SampleShadowMapPCF(shadowMapIndex, projCoords.xy, projCoords.z, bias);
    }

    // -------------------------------------------------------------------
    // Directional light: cascaded shadow maps
    // -------------------------------------------------------------------
    vec3 lightDir = normalize(-light.position_type.xyz);

    float ndotl = dot(N, lightDir);
    if (ndotl <= 0.0)
        return 0.0;

    float bias = max(
        ubo_shadowData.settings.max_bias * (1.0 - ndotl),
        ubo_shadowData.settings.min_bias
    );

    // Normal offset to reduce acne on grazing surfaces
    vec3 shadowSamplePos = worldPos + N * ubo_shadowData.settings.normal_offset;

    // View-space depth for cascade selection
    vec3 camForward = -vec3(
        ubo_globalData.camera.view[0][2],
        ubo_globalData.camera.view[1][2],
        ubo_globalData.camera.view[2][2]
    );
    float viewDepth = dot(worldPos - ubo_globalData.camera.position.xyz, camForward);

    // Select cascade
    uint cascadeIndex;
    if      (viewDepth < light.cascade_splits.x) cascadeIndex = 0u;
    else if (viewDepth < light.cascade_splits.y) cascadeIndex = 1u;
    else if (viewDepth < light.cascade_splits.z) cascadeIndex = 2u;
    else                                          cascadeIndex = 3u;

    // Project into cascade
    vec4 lightSpacePos = light.cascade_matrices[cascadeIndex] * vec4(shadowSamplePos, 1.0);
    vec3 projCoords = lightSpacePos.xyz / lightSpacePos.w;
    projCoords.xy = projCoords.xy * 0.5 + 0.5;

    if (projCoords.x < 0.0 || projCoords.x > 1.0 ||
        projCoords.y < 0.0 || projCoords.y > 1.0 ||
        projCoords.z < 0.0 || projCoords.z > 1.0)
        return 1.0;

    uint shadowMapIndex = uint(light.shadow_params.x) + cascadeIndex;
    float shadow = SampleShadowMapPCF(shadowMapIndex, projCoords.xy, projCoords.z, bias);

    // -------------------------------------------------------------------
    // Blend cascade seam
    // -------------------------------------------------------------------
    const float BLEND_THRESHOLD = 0.95;

    if (cascadeIndex < 3u)
    {
        float prevSplit  = (cascadeIndex == 0u) ? 0.0 : light.cascade_splits[cascadeIndex - 1u];
        float currSplit  = light.cascade_splits[cascadeIndex];
        float range      = currSplit - prevSplit;
        float blendStart = prevSplit + range * BLEND_THRESHOLD;
        float fade       = clamp(
            (viewDepth - blendStart) / (range * (1.0 - BLEND_THRESHOLD)),
            0.0, 1.0
        );

        if (fade > 0.0)
        {
            uint nextIdx = cascadeIndex + 1u;
            vec4 nextLightSpacePos = light.cascade_matrices[nextIdx] * vec4(shadowSamplePos, 1.0);
            vec3 nextCoords = nextLightSpacePos.xyz / nextLightSpacePos.w;
            nextCoords.xy = nextCoords.xy * 0.5 + 0.5;

            if (nextCoords.x >= 0.0 && nextCoords.x <= 1.0 &&
                nextCoords.y >= 0.0 && nextCoords.y <= 1.0 &&
                nextCoords.z >= 0.0 && nextCoords.z <= 1.0)
            {
                uint nextMapIndex = uint(light.shadow_params.x) + nextIdx;
                float nextShadow  = SampleShadowMapPCF(nextMapIndex, nextCoords.xy, nextCoords.z, bias);
                shadow = mix(shadow, nextShadow, fade);
            }
        }
    }

    return shadow;
}

// ================================
// Main PBR Shader
// ================================
void main()
{
    PerObjectData pd = ssbo_objects.data[uint(vObjectIndex)];
    MaterialInfo mat = ssbo_materials.data[pd.material_index];

    // ================================
    // Material Property Sampling
    // ================================

    // Base Color (Albedo)
    int albedoIdx = (mat.base_color > 0) ? mat.base_color : mat.diffuse;
    vec4 albedoSample = SampleTexture(albedoIdx, vUV);

    vec3 albedo = albedoSample.rgb;
    float alpha = albedoSample.a;

    // Alpha test
    if (alpha < 0.1) discard;

    // Metallic
    int metallicIdx = (mat.metalness > 0) ? mat.metalness : -1;
    float metallic = (metallicIdx >= 0) ? SampleTexture(metallicIdx, vUV).r : 0.0;

    // Roughness
    // Note: Specular maps are inverted (high specular = low roughness)
    float roughness;
    int roughnessIdx = (mat.diffuse_roughness > 0) ? mat.diffuse_roughness : -1;
    if (roughnessIdx < 0 && mat.specular > 0) {
        // Legacy specular: invert it to get roughness
        float specular = SampleTexture(mat.specular, vUV).r;
        roughness = 1.0 - specular;
    } else {
        roughness = SampleTexture(roughnessIdx, vUV).r;
    }
    roughness = clamp(roughness, 0.04, 1.0);

    // Ambient Occlusion
    int aoIdx = (mat.ambient_occlusion > 0) ? mat.ambient_occlusion : mat.ambient;
    float ao = SampleTexture(aoIdx, vUV).r;

    // Normal mapping with proper tangent space
    vec3 N = normalize(vNormalWS);
    if (mat.normal > 0 || mat.normal_camera > 0) {
        int normalIdx = (mat.normal_camera > 0) ? mat.normal_camera : mat.normal;
        vec3 normalMap = SampleTexture(normalIdx, vUV).xyz;
        normalMap = normalize(normalMap * 2.0 - 1.0); // Convert from [0,1] to [-1,1]

        // Build TBN matrix for tangent space to world space transformation
        vec3 T = normalize(vTangentWS);
        vec3 B = normalize(vBitangentWS);
        vec3 Nbase = normalize(vNormalWS);
        mat3 TBN = mat3(T, B, Nbase);

        // Transform normal from tangent space to world space
        N = normalize(TBN * normalMap);
    }

    // Emission
    vec3 emission = vec3(0.0);
    if (mat.emissive > 0 || mat.emission_color > 0) {
        int emitIdx = (mat.emission_color > 0) ? mat.emission_color : mat.emissive;
        emission = SampleTexture(emitIdx, vUV).rgb;
    }

    // ================================
    // Lighting Calculations
    // ================================

    vec3 V = normalize(ubo_globalData.camera.position - vPosWS);

    // F0 represents the base reflectivity (0.04 for dielectrics, albedo for metals)
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);

    // Accumulated radiance
    vec3 Lo = vec3(0.0);

    // Calculate lighting for each active light source
    uint numLights = min(ubo_lightData.light_count, MAX_LIGHTS);
    for (uint i = 0u; i < numLights; i++)
    {
        LightInfo light = ubo_lightData.lights[i];

        // Skip disabled lights
        if (light.cone_attenuation.w < 0.5)
        {
            continue;
        }

        uint lightType = uint(light.position_type.w);

        float shadowFactor = CalculateShadow(light, vPosWS, N, i);

        // Point light
        if (lightType == 0u)
        {
            PointLight pointLight;
            pointLight.position = light.position_type.xyz;
            pointLight.color = light.color_intensity.xyz;
            pointLight.intensity = light.color_intensity.w;
            pointLight.radius = light.direction_range.w;

            Lo += CalculatePointLight(
                pointLight,
                N,
                V,
                vPosWS,
                albedo,
                metallic,
                roughness,
                F0
            ) * shadowFactor;
        }
        // Directional light
        else if (lightType == 1u)
        {
            DirectionalLight directionalLight;
            directionalLight.position = light.position_type.xyz;
            directionalLight.color = light.color_intensity.xyz;
            directionalLight.intensity = light.color_intensity.w;
            directionalLight.direction = light.position_type.xyz;

            Lo += CalculateDirectionalLight(
                directionalLight,
                N,
                V,
                albedo,
                metallic,
                roughness,
                F0
            ) * shadowFactor;
        }
        // Spot light
        else if (lightType == 2u)
        {
            SpotLight spotLight;
            spotLight.position = light.position_type.xyz;
            spotLight.color = light.color_intensity.xyz;
            spotLight.intensity = light.color_intensity.w;
            spotLight.direction = light.direction_range.xyz;
            spotLight.innerCutoff = light.cone_attenuation.x;
            spotLight.outerCutoff = light.cone_attenuation.y;

            Lo += CalculateSpotLight(
                spotLight,
                N,
                V,
                vPosWS,
                albedo,
                metallic,
                roughness,
                F0
            ) * shadowFactor;
        }
    }

    // ================================
    // Ambient Lighting (world/environment light)
    // ================================

    vec3 ambientColor = vec3(0.03); // Default ambient color (can be overridden by GI)
    float ambientIntensity = 0.5; // Default ambient intensity
    vec3 ambient = ambientColor * ambientIntensity * albedo * ao;

    vec3 gi = vec3(0.0);

    if (ubo_giData.settings.enabled != 0u)
    {
        vec2 screenUV = gl_FragCoord.xy / vec2(textureSize(globalIlluminationTexture, 0));
        gi = texture(globalIlluminationTexture, screenUV).rgb;
    }

    // Final color
    vec3 color = ambient + Lo + gi + emission;

    // ================================
    // Tone Mapping & Gamma Correction
    // ================================

    color = ACESFilm(color);
    color = LinearToSRGB(color);

    // ================================
    // HELLENGINE_EDITOR Debug Visualization
    // ================================
    switch (pc.debug_view_mode)
    {
        case 0u: break;  // Normal rendering (keep PBR result)
        case 1u:
        {
            // Wireframe mode - handled by pipeline polygon mode
            color = vec3(0.2, 0.8, 1.0);  // Light blue wireframe
            break;
        };
        case 2u:
        {
            // UV visualization - show UVs as red/green gradient
            color = vec3(fract(vUV.x), fract(vUV.y), 0.0);
            break;
        };
        case 3u:
        {
            // Normal visualization (world-space normals mapped to RGB)
            color = vNormalWS * 0.5 + 0.5;
            break;
        };
        case 4u:
        {
            color = vec3(0.0);
            bool anyLight = false;

            for (uint i = 0u; i < ubo_lightData.light_count; ++i)
            {
                LightInfo light = ubo_lightData.lights[i];
                if (light.shadow_params.w < 0.5)
                    continue;

                float lightType = light.position_type.w;

                // ----------------------------------------------------------
                // Directional light: show cascade with tint per cascade
                // ----------------------------------------------------------
                if (lightType == 1.0)
                {
                    vec3 camForward = -vec3(
                        ubo_globalData.camera.view[0][2],
                        ubo_globalData.camera.view[1][2],
                        ubo_globalData.camera.view[2][2]
                    );
                    float viewDepth = dot(vPosWS - ubo_globalData.camera.position.xyz, camForward);

                    uint cascadeIndex;
                    if      (viewDepth < light.cascade_splits.x) cascadeIndex = 0u;
                    else if (viewDepth < light.cascade_splits.y) cascadeIndex = 1u;
                    else if (viewDepth < light.cascade_splits.z) cascadeIndex = 2u;
                    else                                          cascadeIndex = 3u;

                    vec4 lightSpacePos = light.cascade_matrices[cascadeIndex] * vec4(vPosWS, 1.0);
                    vec3 projCoords = lightSpacePos.xyz / lightSpacePos.w;
                    projCoords.xy = projCoords.xy * 0.5 + 0.5;

                    if (projCoords.x >= 0.0 && projCoords.x <= 1.0 &&
                        projCoords.y >= 0.0 && projCoords.y <= 1.0 &&
                        projCoords.z >= 0.0 && projCoords.z <= 1.0)
                    {
                        uint shadowMapIndex = uint(light.shadow_params.x) + cascadeIndex;
                        float depth = texture(shadowMaps[shadowMapIndex], projCoords.xy).r;

                        vec3 cascadeTint;
                        if      (cascadeIndex == 0u) cascadeTint = vec3(1.0, 0.4, 0.4); // red
                        else if (cascadeIndex == 1u) cascadeTint = vec3(0.4, 1.0, 0.4); // green
                        else if (cascadeIndex == 2u) cascadeTint = vec3(0.4, 0.4, 1.0); // blue
                        else                         cascadeTint = vec3(1.0, 1.0, 0.4); // yellow

                        color += vec3(depth) * cascadeTint;
                        anyLight = true;
                    }
                    else
                    {
                        color += vec3(1.0, 0.0, 1.0); // magenta = outside cascade
                        anyLight = true;
                    }
                }
                // ----------------------------------------------------------
                // Spot / Point light: single shadow map, tint cyan/orange
                // ----------------------------------------------------------
                else
                {
                    vec4 lightSpacePos = light.shadow_matrix * vec4(vPosWS, 1.0);
                    vec3 projCoords = lightSpacePos.xyz / lightSpacePos.w;
                    projCoords.xy = projCoords.xy * 0.5 + 0.5;

                    if (projCoords.x >= 0.0 && projCoords.x <= 1.0 &&
                        projCoords.y >= 0.0 && projCoords.y <= 1.0 &&
                        projCoords.z >= 0.0 && projCoords.z <= 1.0)
                    {
                        uint shadowMapIndex = uint(light.shadow_params.x);
                        float depth = texture(shadowMaps[shadowMapIndex], projCoords.xy).r;

                        vec3 lightTint = (lightType == 2.0)
                            ? vec3(0.4, 1.0, 1.0)   // cyan   = spot
                            : vec3(1.0, 0.6, 0.2);  // orange = point

                        color += vec3(depth) * lightTint;
                        anyLight = true;
                    }
                    else
                    {
                        color += vec3(0.5, 0.0, 0.5); // dark magenta = outside frustum
                        anyLight = true;
                    }
                }
            }

            // No shadow-casting lights found
            if (!anyLight)
                color = vec3(0.2);

            break;
        }
    }

    outFragColor = vec4(color, alpha);
    outPickId = uint(pd.entity_id);
}
