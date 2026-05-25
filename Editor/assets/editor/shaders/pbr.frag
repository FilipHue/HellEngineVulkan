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
    vec4 debug;
} ubo_giData;

// Shadows data
layout(set = 0, binding = 3) uniform ShadowData {
    ShadowSettings settings;
} ubo_shadowData;

// Global illumination texture
layout(set = 0, binding = 4) uniform sampler2D globalIlluminationTexture;

// Material data
layout(set = 1, binding = 0, std430) readonly buffer MaterialSSBO {
    MaterialInfo data[];
} ssbo_materials;

// Per-object data
layout(set = 2, binding = 0, std430) readonly buffer PerObjectSSBO {
    PerObjectData data[];
} ssbo_objects;

// Set 3: shadow maps, point shadow cubemaps, bindless textures
// binding 0 — 2D shadow maps (directional cascades + spot)
// binding 1 — cube shadow maps (point lights)
// binding 2 — bindless material textures (variable count, must be last)
layout(set = 3, binding = 0) uniform sampler2D     shadowMaps[MAX_SHADOW_MAPS];
layout(set = 3, binding = 1) uniform samplerCube   pointShadowMaps[MAX_POINT_SHADOW_MAPS];
layout(set = 3, binding = 2) uniform sampler2D     textures[];

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
    vec2  texelSize = 1.0 / vec2(textureSize(shadowMaps[nonuniformEXT(shadowMapIndex)], 0));
    int   radius    = int(ubo_shadowData.settings.pcf_samples);
    float result    = 0.0;
    int   count     = 0;

    for (int x = -radius; x <= radius; ++x)
    {
        for (int y = -radius; y <= radius; ++y)
        {
            // Use a fixed texel-aligned offset — no softness multiplier
            // Softness scaling was causing sub-texel sampling which flickers
            vec2 offset = vec2(x, y) * texelSize * max(1.0, floor(ubo_shadowData.settings.softness));
            float closestDepth = texture(shadowMaps[nonuniformEXT(shadowMapIndex)], uv + offset).r;
            result += ((currentDepth - bias) > closestDepth) ? 0.0 : 1.0;
            ++count;
        }
    }

    return result / float(count);
}

float SamplePointShadow(uint cubemapIndex, vec3 fragToLight, float range, float minBias, float maxBias, vec3 N)
{
    vec3  toLight  = normalize(-fragToLight);
    float ndotl    = dot(N, toLight);
    if (ndotl <= 0.0) return 0.0;

    // Stored depth is linear [0,1] written as length/farPlane in shadow_depth_point.frag
    float currentDepth = length(fragToLight) / range;
    float closestDepth = texture(pointShadowMaps[nonuniformEXT(cubemapIndex)], fragToLight).r;

    float bias = max(maxBias * (1.0 - ndotl), minBias) / range;

    return (currentDepth - bias) > closestDepth ? 0.0 : 1.0;
}

float CalculateShadow(LightInfo light, vec3 worldPos, vec3 normal, uint lightIndex)
{
    if (ubo_shadowData.settings.enabled == 0u || light.shadow_params.w < 0.5)
        return 1.0;

    float lightType = light.position_type.w;
    vec3 N = normalize(normal);

    // ── Point light: sample cubemap ────────────────────────────────────
    if (lightType == 0.0)
    {
        vec3 fragToLight  = worldPos - light.position_type.xyz;
        uint cubemapIndex = uint(light.shadow_params.x);

        return SamplePointShadow(
            cubemapIndex,
            fragToLight,
            light.direction_range.w,
            ubo_shadowData.settings.min_bias,
            ubo_shadowData.settings.max_bias,
            N
        );
    }

    // ── Spot light ─────────────────────────────────────────────────────
    if (lightType != 1.0)
    {
        vec4 lightSpacePos = light.shadow_matrix * vec4(worldPos, 1.0);
        vec3 projCoords    = lightSpacePos.xyz / lightSpacePos.w;
        projCoords.xy      = projCoords.xy * 0.5 + 0.5;

        if (projCoords.x < 0.0 || projCoords.x > 1.0 ||
            projCoords.y < 0.0 || projCoords.y > 1.0 ||
            projCoords.z < 0.0 || projCoords.z > 1.0)
            return 1.0;

        vec3 lightDir = normalize(light.direction_range.xyz);

        float bias = max(
            ubo_shadowData.settings.max_bias * (1.0 - dot(N, lightDir)),
            ubo_shadowData.settings.min_bias
        );

        uint shadowMapIndex = uint(light.shadow_params.x);
        return SampleShadowMapPCF(shadowMapIndex, projCoords.xy, projCoords.z, bias);
    }

    // ── Directional light: cascaded shadow maps ─────────────────────────
    vec3  lightDir = normalize(-light.position_type.xyz);
    float ndotl    = dot(N, lightDir);
    if (ndotl <= 0.0)
        return 0.0;

    float bias = max(
        ubo_shadowData.settings.max_bias * (1.0 - ndotl),
        ubo_shadowData.settings.min_bias
    );

    vec3 shadowSamplePos = worldPos + N * ubo_shadowData.settings.normal_offset;

    vec3 camForward = -vec3(
        ubo_globalData.camera.view[0][2],
        ubo_globalData.camera.view[1][2],
        ubo_globalData.camera.view[2][2]
    );
    float viewDepth = dot(worldPos - ubo_globalData.camera.position.xyz, camForward);

    uint cascadeIndex;
    if      (viewDepth < light.cascade_splits.x) cascadeIndex = 0u;
    else if (viewDepth < light.cascade_splits.y) cascadeIndex = 1u;
    else if (viewDepth < light.cascade_splits.z) cascadeIndex = 2u;
    else                                          cascadeIndex = 3u;

    vec4 lightSpacePos = light.cascade_matrices[cascadeIndex] * vec4(shadowSamplePos, 1.0);
    vec3 projCoords    = lightSpacePos.xyz / lightSpacePos.w;
    projCoords.xy      = projCoords.xy * 0.5 + 0.5;

    if (projCoords.x < 0.0 || projCoords.x > 1.0 ||
        projCoords.y < 0.0 || projCoords.y > 1.0 ||
        projCoords.z < 0.0 || projCoords.z > 1.0)
        return 1.0;

    uint  shadowMapIndex = uint(light.shadow_params.x) + cascadeIndex;
    float shadow         = SampleShadowMapPCF(shadowMapIndex, projCoords.xy, projCoords.z, bias);

    // Blend cascade seam
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
            uint nextIdx           = cascadeIndex + 1u;
            vec4 nextLightSpacePos = light.cascade_matrices[nextIdx] * vec4(shadowSamplePos, 1.0);
            vec3 nextCoords        = nextLightSpacePos.xyz / nextLightSpacePos.w;
            nextCoords.xy          = nextCoords.xy * 0.5 + 0.5;

            if (nextCoords.x >= 0.0 && nextCoords.x <= 1.0 &&
                nextCoords.y >= 0.0 && nextCoords.y <= 1.0 &&
                nextCoords.z >= 0.0 && nextCoords.z <= 1.0)
            {
                uint  nextMapIndex = uint(light.shadow_params.x) + nextIdx;
                float nextShadow   = SampleShadowMapPCF(nextMapIndex, nextCoords.xy, nextCoords.z, bias);
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
    PerObjectData pd  = ssbo_objects.data[uint(vObjectIndex)];
    MaterialInfo  mat = ssbo_materials.data[pd.material_index];

    // ================================
    // Material Property Sampling
    // ================================

    int  albedoIdx    = (mat.base_color > 0) ? mat.base_color : mat.diffuse;
    vec4 albedoSample = SampleTexture(albedoIdx, vUV);

    vec3  albedo = albedoSample.rgb;
    float alpha  = albedoSample.a;

    if (alpha < 0.1) discard;

    int   metallicIdx = (mat.metalness > 0) ? mat.metalness : -1;
    float metallic    = (metallicIdx >= 0) ? SampleTexture(metallicIdx, vUV).r : 0.0;

    float roughness;
    int   roughnessIdx = (mat.diffuse_roughness > 0) ? mat.diffuse_roughness : -1;
    if (roughnessIdx < 0 && mat.specular > 0)
    {
        float specular = SampleTexture(mat.specular, vUV).r;
        roughness = 1.0 - specular;
    }
    else
    {
        roughness = SampleTexture(roughnessIdx, vUV).r;
    }
    roughness = clamp(roughness, 0.04, 1.0);

    int   aoIdx = (mat.ambient_occlusion > 0) ? mat.ambient_occlusion : mat.ambient;
    float ao    = SampleTexture(aoIdx, vUV).r;

    vec3 N = normalize(vNormalWS);
    if (mat.normal > 0 || mat.normal_camera > 0)
    {
        int  normalIdx = (mat.normal_camera > 0) ? mat.normal_camera : mat.normal;
        vec3 normalMap = SampleTexture(normalIdx, vUV).xyz;
        normalMap      = normalize(normalMap * 2.0 - 1.0);

        vec3 T    = normalize(vTangentWS);
        vec3 B    = normalize(vBitangentWS);
        vec3 Nb   = normalize(vNormalWS);
        mat3 TBN  = mat3(T, B, Nb);
        N         = normalize(TBN * normalMap);
    }

    vec3 emission = vec3(0.0);
    if (mat.emissive > 0 || mat.emission_color > 0)
    {
        int emitIdx = (mat.emission_color > 0) ? mat.emission_color : mat.emissive;
        emission = SampleTexture(emitIdx, vUV).rgb;
    }

    // ================================
    // Lighting Calculations
    // ================================

    vec3 V  = normalize(ubo_globalData.camera.position - vPosWS);
    vec3 F0 = mix(vec3(0.04), albedo, metallic);
    vec3 Lo = vec3(0.0);

    uint numLights = min(ubo_lightData.light_count, MAX_LIGHTS);
    for (uint i = 0u; i < numLights; i++)
    {
        LightInfo light = ubo_lightData.lights[i];

        if (light.cone_attenuation.w < 0.5)
            continue;

        uint  lightType    = uint(light.position_type.w);
        float shadowFactor = CalculateShadow(light, vPosWS, N, i);

        if (lightType == 0u)
        {
            PointLight pointLight;
            pointLight.position  = light.position_type.xyz;
            pointLight.color     = light.color_intensity.xyz;
            pointLight.intensity = light.color_intensity.w;
            pointLight.radius    = light.direction_range.w;

            Lo += CalculatePointLight(pointLight, N, V, vPosWS, albedo, metallic, roughness, F0) * shadowFactor;
        }
        else if (lightType == 1u)
        {
            DirectionalLight directionalLight;
            directionalLight.position  = light.position_type.xyz;
            directionalLight.color     = light.color_intensity.xyz;
            directionalLight.intensity = light.color_intensity.w;
            directionalLight.direction = light.position_type.xyz;

            Lo += CalculateDirectionalLight(directionalLight, N, V, albedo, metallic, roughness, F0) * shadowFactor;
        }
        else if (lightType == 2u)
        {
            SpotLight spotLight;
            spotLight.position    = light.position_type.xyz;
            spotLight.color       = light.color_intensity.xyz;
            spotLight.intensity   = light.color_intensity.w;
            spotLight.direction   = light.direction_range.xyz;
            spotLight.innerCutoff = light.cone_attenuation.x;
            spotLight.outerCutoff = light.cone_attenuation.y;

            Lo += CalculateSpotLight(spotLight, N, V, vPosWS, albedo, metallic, roughness, F0) * shadowFactor;
        }
    }

    // ================================
    // Ambient + GI
    // ================================

    vec3  ambientColor     = vec3(0.03);
    float ambientIntensity = 0.5;
    vec3  ambient          = ambientColor * ambientIntensity * albedo * ao;

    vec3 gi = vec3(0.0);
    if (ubo_giData.settings.enabled != 0u)
    {
        vec2 screenUV = gl_FragCoord.xy / vec2(textureSize(globalIlluminationTexture, 0));
        gi = texture(globalIlluminationTexture, screenUV).rgb;
    }

    vec3 color = ambient + Lo + gi + emission;

    // ================================
    // Tone Mapping & Gamma Correction
    // ================================

    color = ACESFilm(color);
    color = LinearToSRGB(color);

    // ================================
    // Debug Visualization
    // ================================
    switch (pc.debug_view_mode)
    {
        case 0u: break;
        case 1u:
        {
            color = vec3(0.2, 0.8, 1.0);
            break;
        };
        case 2u:
        {
            color = vec3(fract(vUV.x), fract(vUV.y), 0.0);
            break;
        };
        case 3u:
        {
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
                    vec3 projCoords    = lightSpacePos.xyz / lightSpacePos.w;
                    projCoords.xy      = projCoords.xy * 0.5 + 0.5;

                    if (projCoords.x >= 0.0 && projCoords.x <= 1.0 &&
                        projCoords.y >= 0.0 && projCoords.y <= 1.0 &&
                        projCoords.z >= 0.0 && projCoords.z <= 1.0)
                    {
                        uint  shadowMapIndex = uint(light.shadow_params.x) + cascadeIndex;
                        float depth          = texture(shadowMaps[shadowMapIndex], projCoords.xy).r;
                        float remappedDepth  = pow(depth, 0.5);

                        vec3 cascadeTint;
                        if      (cascadeIndex == 0u) cascadeTint = vec3(1.0, 0.2, 0.2);
                        else if (cascadeIndex == 1u) cascadeTint = vec3(0.2, 1.0, 0.2);
                        else if (cascadeIndex == 2u) cascadeTint = vec3(0.2, 0.2, 1.0);
                        else                         cascadeTint = vec3(1.0, 1.0, 0.2);

                        color = mix(vec3(0.0), cascadeTint, remappedDepth);

                        float borderWidth = 0.02;
                        if (projCoords.x < borderWidth || projCoords.x > 1.0 - borderWidth ||
                            projCoords.y < borderWidth || projCoords.y > 1.0 - borderWidth)
                            color = cascadeTint;

                        if (projCoords.z > depth + 0.001)
                            color *= 0.3;

                        anyLight = true;
                    }
                    else
                    {
                        color    = vec3(1.0, 0.0, 1.0);
                        anyLight = true;
                    }
                }
                else if (lightType == 2.0)
                {
                    vec4 lightSpacePos = light.shadow_matrix * vec4(vPosWS, 1.0);
                    vec3 projCoords    = lightSpacePos.xyz / lightSpacePos.w;
                    projCoords.xy      = projCoords.xy * 0.5 + 0.5;

                    if (projCoords.x >= 0.0 && projCoords.x <= 1.0 &&
                        projCoords.y >= 0.0 && projCoords.y <= 1.0 &&
                        projCoords.z >= 0.0 && projCoords.z <= 1.0)
                    {
                        uint  shadowMapIndex = uint(light.shadow_params.x);
                        float depth          = texture(shadowMaps[shadowMapIndex], projCoords.xy).r;
                        float remappedDepth  = pow(depth, 0.5);
                        color    = mix(vec3(0.0), vec3(0.2, 1.0, 1.0), remappedDepth);
                        anyLight = true;
                    }
                    else
                    {
                        color    = vec3(0.7, 0.0, 0.7);
                        anyLight = true;
                    }
                }
                else if (lightType == 0.0)
                {
                    vec3  fragToLight  = vPosWS - light.position_type.xyz;
                    uint  cubemapIndex = uint(light.shadow_params.x);
                    float depth        = texture(pointShadowMaps[cubemapIndex], fragToLight).r;
                    color    = mix(vec3(0.0), vec3(1.0, 0.5, 0.1), pow(depth, 0.5));
                    anyLight = true;
                }
            }

            if (!anyLight)
            {
                float grid = mod(floor(vPosWS.x) + floor(vPosWS.z), 2.0);
                color = vec3(0.15 + 0.05 * grid);
            }

            break;
        }
    }

    outFragColor = vec4(color, alpha);
    outPickId    = uint(pd.entity_id);
}
