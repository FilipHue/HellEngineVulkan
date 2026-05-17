#version 460

#include "includes/common.glsl"

#extension GL_ARB_shader_draw_parameters : require

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inUV;
layout(location = 3) in vec3 inNormal;
layout(location = 4) in vec3 inTangent;
layout(location = 5) in vec3 inBitangent;

layout(location = 0) out vec3 vColor;
layout(location = 1) out vec2 vUV;
layout(location = 2) out vec3 vPosWS;
layout(location = 3) out vec3 vNormalWS;
layout(location = 4) out vec3 vTangentWS;
layout(location = 5) out vec3 vBitangentWS;
layout(location = 6) out flat int vObjectIndex;

layout(set = 0, binding = 0) uniform GlobalData {
    CameraData camera;
    WorldData world;
} ubo_globalData;

layout(set = 2, binding = 0, std430) readonly buffer PerObjectSSBO {
    PerObjectData data[];
} ssbo_objects;

void main()
{
    uint objectIndex = gl_DrawIDARB;
    vObjectIndex = int(objectIndex);

    PerObjectData pd = ssbo_objects.data[objectIndex];

    // World-space position
    vec4 posWS = pd.model * vec4(inPosition, 1.0);
    vPosWS = posWS.xyz;

    // Compute normal matrix (transpose of inverse for correct non-uniform scale)
    mat3 normalMat = mat3(transpose(inverse(pd.model)));

    // Transform normal, tangent, and bitangent to world space
    vNormalWS = normalize(normalMat * inNormal);
    vTangentWS = normalize(normalMat * inTangent);
    vBitangentWS = normalize(normalMat * inBitangent);

    // Gram-Schmidt orthogonalization to ensure perpendicularity
    vTangentWS = normalize(vTangentWS - dot(vTangentWS, vNormalWS) * vNormalWS);
    float handedness = dot(cross(vNormalWS, vTangentWS), vBitangentWS) < 0.0 ? -1.0 : 1.0;
    vBitangentWS = normalize(cross(vNormalWS, vTangentWS) * handedness);

    // Pass-through
    vColor = inColor;
    vUV = inUV;

    // Clip-space position
    gl_Position = ubo_globalData.camera.projection * ubo_globalData.camera.view * posWS;
}
