// =========================
// G-buffer Fragment Shader
// =========================
#version 460

#extension GL_EXT_nonuniform_qualifier : enable

// Includes
#include "includes/common.glsl"
#include "includes/pbr_common.glsl"
#include "includes/light.glsl"
#include "includes/shadow.glsl"
#include "includes/global_illumination.glsl"

// Inputs
layout(location = 0) in vec3 vColor;
layout(location = 1) in vec2 vUV;
layout(location = 2) in vec3 vPosWS;
layout(location = 3) in vec3 vNormalWS;
layout(location = 4) in vec3 vTangentWS;
layout(location = 5) in vec3 vBitangentWS;
layout(location = 6) in flat int vObjectIndex;

// Outputs
layout(location = 0) out vec4 outPosition;
layout(location = 1) out vec4 outNormal;
layout(location = 2) out vec4 outAlbedoAO;
layout(location = 3) out vec4 outLighting;   // <-- new: raw Lo for GI

// ── Set 0 ──────────────────────────────────────────────────────────────
layout(set = 0, binding = 0) uniform GlobalData
{
    CameraData camera;
    WorldData  world;
} ubo_globalData;

layout(set = 0, binding = 1) uniform LightData
{
    uint      light_count;
    uint      _pad0;
    uint      _pad1;
    uint      _pad2;
    LightInfo lights[MAX_LIGHTS];
} ubo_lightData;

layout(set = 0, binding = 2) uniform GlobalIlluminationData
{
    GlobalIlluminationSettings settings;
    vec4 debug;
} ubo_giData;

layout(set = 0, binding = 3) uniform ShadowData
{
    ShadowSettings settings;
} ubo_shadowData;

// ── Set 1 / 2 ──────────────────────────────────────────────────────────
layout(set = 1, binding = 0, std430) readonly buffer MaterialSSBO
{
    MaterialInfo data[];
} ssbo_materials;

layout(set = 2, binding = 0, std430) readonly buffer PerObjectSSBO
{
    PerObjectData data[];
} ssbo_objects;

// ── Set 3 ──────────────────────────────────────────────────────────────
layout(set = 3, binding = 0) uniform sampler2D shadowMaps[MAX_SHADOW_MAPS];
layout(set = 3, binding = 1) uniform sampler2D textures[];

// ══════════════════════════════════════════════════════════════════════
// Helpers (mirror of pbr.frag — keep in sync or move to a shared include)
// ══════════════════════════════════════════════════════════════════════

vec4 SampleTexture(int index, vec2 uv)
{
    return texture(textures[nonuniformEXT(uint(index))], uv);
}

vec3 GetMaterialNormal(MaterialInfo mat)
{
    vec3 N = normalize(vNormalWS);
    if (mat.normal > 0 || mat.normal_camera > 0)
    {
        int normalIdx = (mat.normal_camera > 0) ? mat.normal_camera : mat.normal;
        vec3 normalMap = normalize(SampleTexture(normalIdx, vUV).xyz * 2.0 - 1.0);
        vec3 T  = normalize(vTangentWS);
        vec3 B  = normalize(vBitangentWS);
        vec3 Nb = normalize(vNormalWS);
        N = normalize(mat3(T, B, Nb) * normalMap);
    }
    return N;
}

float GetMaterialAO(MaterialInfo mat)
{
    int aoIdx = (mat.ambient_occlusion > 0) ? mat.ambient_occlusion : mat.ambient;
    return (aoIdx > 0) ? SampleTexture(aoIdx, vUV).r : 1.0;
}

// ── Shadow (same PCF logic as pbr.frag) ────────────────────────────────

float SampleShadowMapPCF(uint shadowMapIndex, vec2 uv, float currentDepth, float bias)
{
    vec2 texelSize = 1.0 / vec2(textureSize(shadowMaps[nonuniformEXT(shadowMapIndex)], 0));
    int  radius    = int(ubo_shadowData.settings.pcf_samples);
    float result   = 0.0;
    int   count    = 0;
    for (int x = -radius; x <= radius; ++x)
    {
        for (int y = -radius; y <= radius; ++y)
        {
            vec2  offset       = vec2(x, y) * texelSize * ubo_shadowData.settings.softness;
            float closestDepth = texture(shadowMaps[nonuniformEXT(shadowMapIndex)], uv + offset).r;
            result += ((currentDepth - bias) > closestDepth) ? 0.0 : 1.0;
            ++count;
        }
    }
    return result / float(count);
}

float CalculateShadow(LightInfo light, vec3 worldPos, vec3 normal)
{
    if (ubo_shadowData.settings.enabled == 0u || light.shadow_params.w < 0.5)
        return 1.0;

    float lightType = light.position_type.w;
    vec3  N         = normalize(normal);

    // ── Spot / Point ───────────────────────────────────────────────────
    if (lightType != 1.0)
    {
        vec4  lsp        = light.shadow_matrix * vec4(worldPos, 1.0);
        vec3  proj       = lsp.xyz / lsp.w;
        proj.xy          = proj.xy * 0.5 + 0.5;

        if (any(lessThan(proj, vec3(0.0))) || any(greaterThan(proj, vec3(1.0))))
            return 1.0;

        vec3  lightDir   = (lightType == 2.0)
            ? normalize(light.direction_range.xyz)
            : normalize(light.position_type.xyz - worldPos);
        float bias       = max(ubo_shadowData.settings.max_bias * (1.0 - dot(N, lightDir)),
                               ubo_shadowData.settings.min_bias);

        return SampleShadowMapPCF(uint(light.shadow_params.x), proj.xy, proj.z, bias);
    }

    // ── Directional / CSM ─────────────────────────────────────────────
    vec3  lightDir  = normalize(-light.position_type.xyz);
    float ndotl     = dot(N, lightDir);
    if (ndotl <= 0.0) return 0.0;

    float bias      = max(ubo_shadowData.settings.max_bias * (1.0 - ndotl),
                          ubo_shadowData.settings.min_bias);
    vec3  shadowPos = worldPos + N * ubo_shadowData.settings.normal_offset;

    vec3  camFwd    = -vec3(ubo_globalData.camera.view[0][2],
                            ubo_globalData.camera.view[1][2],
                            ubo_globalData.camera.view[2][2]);
    float viewDepth = dot(worldPos - ubo_globalData.camera.position.xyz, camFwd);

    uint cascade;
    if      (viewDepth < light.cascade_splits.x) cascade = 0u;
    else if (viewDepth < light.cascade_splits.y) cascade = 1u;
    else if (viewDepth < light.cascade_splits.z) cascade = 2u;
    else                                          cascade = 3u;

    vec4  lsp   = light.cascade_matrices[cascade] * vec4(shadowPos, 1.0);
    vec3  proj  = lsp.xyz / lsp.w;
    proj.xy     = proj.xy * 0.5 + 0.5;

    if (any(lessThan(proj, vec3(0.0))) || any(greaterThan(proj, vec3(1.0))))
        return 1.0;

    uint  mapIdx = uint(light.shadow_params.x) + cascade;
    float shadow = SampleShadowMapPCF(mapIdx, proj.xy, proj.z, bias);

    // Cascade blend
    const float BLEND = 0.95;
    if (cascade < 3u)
    {
        float prevSplit  = (cascade == 0u) ? 0.0 : light.cascade_splits[cascade - 1u];
        float currSplit  = light.cascade_splits[cascade];
        float range      = currSplit - prevSplit;
        float fade       = clamp((viewDepth - (prevSplit + range * BLEND)) / (range * (1.0 - BLEND)), 0.0, 1.0);

        if (fade > 0.0)
        {
            uint  nextIdx  = cascade + 1u;
            vec4  nlsp     = light.cascade_matrices[nextIdx] * vec4(shadowPos, 1.0);
            vec3  nproj    = nlsp.xyz / nlsp.w;
            nproj.xy       = nproj.xy * 0.5 + 0.5;

            if (all(greaterThanEqual(nproj, vec3(0.0))) && all(lessThanEqual(nproj, vec3(1.0))))
            {
                float ns = SampleShadowMapPCF(uint(light.shadow_params.x) + nextIdx, nproj.xy, nproj.z, bias);
                shadow   = mix(shadow, ns, fade);
            }
        }
    }
    return shadow;
}

// ══════════════════════════════════════════════════════════════════════
// Main
// ══════════════════════════════════════════════════════════════════════
void main()
{
    PerObjectData objectData = ssbo_objects.data[uint(vObjectIndex)];
    MaterialInfo  mat        = ssbo_materials.data[objectData.material_index];

    // ── Albedo / alpha test ────────────────────────────────────────────
    int  albedoIdx    = (mat.base_color > 0) ? mat.base_color : mat.diffuse;
    vec4 albedoSample = SampleTexture(albedoIdx, vUV);
    if (albedoSample.a < 0.1) discard;

    // ── Surface properties ─────────────────────────────────────────────
    vec3  albedo = albedoSample.rgb;
    float ao     = GetMaterialAO(mat);
    vec3  N      = GetMaterialNormal(mat);

    // ── PBR material params ────────────────────────────────────────────
    int   metallicIdx  = (mat.metalness > 0) ? mat.metalness : -1;
    float metallic     = (metallicIdx >= 0) ? SampleTexture(metallicIdx, vUV).r : 0.0;

    float roughness;
    int   roughnessIdx = (mat.diffuse_roughness > 0) ? mat.diffuse_roughness : -1;
    if (roughnessIdx < 0 && mat.specular > 0)
        roughness = 1.0 - SampleTexture(mat.specular, vUV).r;
    else
        roughness = (roughnessIdx >= 0) ? SampleTexture(roughnessIdx, vUV).r : 0.5;
    roughness = clamp(roughness, 0.04, 1.0);

    // ── Lighting loop (produces Lo — raw direct light, no tonemap) ─────
    vec3 V  = normalize(ubo_globalData.camera.position.xyz - vPosWS);
    vec3 F0 = mix(vec3(0.04), albedo, metallic);
    vec3 Lo = vec3(0.0);

    uint numLights = min(ubo_lightData.light_count, MAX_LIGHTS);
    for (uint i = 0u; i < numLights; ++i)
    {
        LightInfo light = ubo_lightData.lights[i];
        if (light.cone_attenuation.w < 0.5) continue;

        float shadow    = CalculateShadow(light, vPosWS, N);
        uint  lightType = uint(light.position_type.w);

        if (lightType == 0u)
        {
            PointLight pl;
            pl.position  = light.position_type.xyz;
            pl.color     = light.color_intensity.xyz;
            pl.intensity = light.color_intensity.w;
            pl.radius    = light.direction_range.w;
            Lo += CalculatePointLight(pl, N, V, vPosWS, albedo, metallic, roughness, F0) * shadow;
        }
        else if (lightType == 1u)
        {
            DirectionalLight dl;
            dl.position  = light.position_type.xyz;
            dl.color     = light.color_intensity.xyz;
            dl.intensity = light.color_intensity.w;
            dl.direction = light.position_type.xyz;
            Lo += CalculateDirectionalLight(dl, N, V, albedo, metallic, roughness, F0) * shadow;
        }
        else if (lightType == 2u)
        {
            SpotLight sl;
            sl.position     = light.position_type.xyz;
            sl.color        = light.color_intensity.xyz;
            sl.intensity    = light.color_intensity.w;
            sl.direction    = light.direction_range.xyz;
            sl.innerCutoff  = light.cone_attenuation.x;
            sl.outerCutoff  = light.cone_attenuation.y;
            Lo += CalculateSpotLight(sl, N, V, vPosWS, albedo, metallic, roughness, F0) * shadow;
        }
    }

    // ── G-buffer outputs ───────────────────────────────────────────────
    outPosition  = vec4(vPosWS, 1.0);
    outNormal    = vec4(N * 0.5 + 0.5, 1.0);
    outAlbedoAO  = vec4(albedo, ao);
    outLighting  = vec4(Lo, 1.0);   // shadow-aware direct light, linear, no tonemap
}
