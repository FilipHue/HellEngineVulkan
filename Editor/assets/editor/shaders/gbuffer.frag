// =========================
// G-buffer Fragment Shader
// =========================
#version 460

#extension GL_EXT_nonuniform_qualifier : enable

// Includes
#include "includes/common.glsl"

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

// Descriptors
// Set 0: Global data
layout(set = 0, binding = 0) uniform GlobalData
{
    CameraData camera;
    WorldData world;
} ubo_globalData;

// Set 1: Material data
layout(set = 1, binding = 0, std430) readonly buffer MaterialSSBO
{
    MaterialInfo data[];
} ssbo_materials;

// Set 2: Per-object data
layout(set = 2, binding = 0, std430) readonly buffer PerObjectSSBO
{
    PerObjectData data[];
} ssbo_objects;

// Set 3: Textures
layout(set = 3, binding = 1) uniform sampler2D textures[];

// Functions
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

        vec3 normalMap = SampleTexture(normalIdx, vUV).xyz;
        normalMap = normalize(normalMap * 2.0 - 1.0);

        vec3 T = normalize(vTangentWS);
        vec3 B = normalize(vBitangentWS);
        vec3 baseN = normalize(vNormalWS);

        mat3 TBN = mat3(T, B, baseN);
        N = normalize(TBN * normalMap);
    }

    return N;
}

float GetMaterialAO(MaterialInfo mat)
{
    int aoIdx = (mat.ambient_occlusion > 0)
        ? mat.ambient_occlusion
        : mat.ambient;

    if (aoIdx > 0)
    {
        return SampleTexture(aoIdx, vUV).r;
    }

    return 1.0;
}

void main()
{
    PerObjectData objectData = ssbo_objects.data[uint(vObjectIndex)];
    MaterialInfo mat = ssbo_materials.data[objectData.material_index];

    int albedoIdx = (mat.base_color > 0) ? mat.base_color : mat.diffuse;
    vec4 albedoSample = SampleTexture(albedoIdx, vUV);

    if (albedoSample.a < 0.1)
    {
        discard;
    }

    vec3 N = GetMaterialNormal(mat);
    float ao = GetMaterialAO(mat);

    outPosition = vec4(vPosWS, 1.0);
    outNormal = vec4(N * 0.5 + 0.5, 1.0);
    outAlbedoAO = vec4(albedoSample.rgb, ao);
}
