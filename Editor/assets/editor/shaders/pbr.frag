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
layout(set = 3, binding = 0) uniform sampler2D shadowMaps[MAX_LIGHTS];
layout(set = 3, binding = 1) uniform sampler2D textures[];


layout(constant_id = 0) const uint HELLENGINE_EDITOR = 0;

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

// Calculate shadow factor for a light (0.0 = fully shadowed, 1.0 = fully lit)
float CalculateShadow(LightInfo light, vec3 worldPos, vec3 normal, uint lightIndex)
{
    // Early exit if shadows disabled globally or for this light
    if (ubo_shadowData.settings.enabled == 0u || light.shadow_params.w < 0.5)
        return 1.0;

    // Transform world position to light space
    vec4 lightSpacePos = light.shadow_matrix * vec4(worldPos, 1.0);

    // Perspective divide
    vec3 projCoords = lightSpacePos.xyz / lightSpacePos.w;

    // Transform to [0,1] range
    projCoords.xy = projCoords.xy * 0.5 + 0.5;

    // Sample shadow map
    float shadowMapDepth = texture(shadowMaps[lightIndex], projCoords.xy).r;

    // Outside shadow map bounds - no shadow
    if (projCoords.x < 0.0 || projCoords.x > 1.0 ||
        projCoords.y < 0.0 || projCoords.y > 1.0 ||
        projCoords.z > 1.0)
        return 1.0;

    // Clamp z to valid range (allow 0.0)
    if (projCoords.z < 0.0)
        projCoords.z = 0.0;

    float currentDepth = projCoords.z;

    // Apply bias to reduce shadow acne
    vec3 lightDir;
    float lightType = light.position_type.w;
    if (lightType == 1.0) {
        lightDir = normalize(-light.position_type.xyz);
    } else if (lightType == 2.0) {
        lightDir = normalize(light.direction_range.xyz);
    } else {
        lightDir = normalize(light.position_type.xyz - worldPos);
    }

    float bias = max(ubo_shadowData.settings.max_bias * (1.0 - dot(normal, lightDir)), ubo_shadowData.settings.min_bias);

    // Standard depth comparison
    float shadow = (currentDepth - bias) > shadowMapDepth ? 0.0 : 1.0;

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

    vec3 ambientColor = ubo_globalData.world.ambient_color_intensity.xyz;
    float ambientIntensity = ubo_globalData.world.ambient_color_intensity.w;
    vec3 ambient = ambientColor * ambientIntensity * albedo * ao;

    vec2 screenUV = gl_FragCoord.xy / vec2(textureSize(globalIlluminationTexture, 0));
    vec3 gi = texture(globalIlluminationTexture, screenUV).rgb;

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
    if (HELLENGINE_EDITOR == 1u)
    {
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
                // Shadow Map visualization - show depth from first shadow-casting light
                // Find first shadow-casting light
                for (uint i = 0u; i < ubo_lightData.light_count; ++i)
                {
                    LightInfo light = ubo_lightData.lights[i];
                    if (light.shadow_params.w >= 0.5)  // If light casts shadows
                    {
                        // Transform to light space
                        vec4 lightSpacePos = light.shadow_matrix * vec4(vPosWS, 1.0);
                        vec3 projCoords = lightSpacePos.xyz / lightSpacePos.w;
                        projCoords = projCoords * 0.5 + 0.5;

                        // Visualize shadow map sampling in light-space bounds
                        if (projCoords.x >= 0.0 && projCoords.x <= 1.0 &&
                            projCoords.y >= 0.0 && projCoords.y <= 1.0 &&
                            projCoords.z >= 0.0 && projCoords.z <= 1.0)
                        {
                            float shadowDepth = texture(shadowMaps[i], projCoords.xy).r;
                            // Grayscale depth + blue tint when inside valid projection bounds
                            color = vec3(shadowDepth);
                            color.b = max(color.b, 0.2);
                        }
                        else
                        {
                            // Outside projection bounds
                            color = vec3(1.0, 0.0, 1.0);
                        }

                        break;  // Only visualize first shadow-casting light
                    }
                }
            }
        }
    }

    outFragColor = vec4(color, alpha);
    outPickId = uint(pd.entity_id);
}
