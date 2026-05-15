#version 460

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

layout(set = 0, binding = 0) uniform GlobalUBO {
    mat4 proj;
    mat4 view;
    vec3 camera_position;
    float _pad0;
    vec4 ambient_color_intensity;  // xyz = color, w = intensity
} ubo;

struct PerObjectData {
    mat4 model;
    uint material_index;
    int  entity_id;
};

layout(set = 2, binding = 0, std430) readonly buffer PerObjectSSBO {
    PerObjectData obj[];
};

void main()
{
    uint objectIndex = gl_DrawIDARB;
    vObjectIndex = int(objectIndex);

    PerObjectData pd = obj[objectIndex];

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
    vBitangentWS = cross(vNormalWS, vTangentWS);

    // Pass-through
    vColor = inColor;
    vUV = inUV;

    // Clip-space position
    gl_Position = ubo.proj * ubo.view * posWS;
}
