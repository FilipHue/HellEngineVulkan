// ================================
// Vulkan GLSL Vertex Shader (instanced)
// ================================
#version 460

// Needed for gl_BaseInstanceARB (maps to Vulkan firstInstance)
#extension GL_ARB_shader_draw_parameters : require

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inUV;
layout(location = 3) in vec3 inNormal;

layout(location = 0) out vec3 vColor;
layout(location = 1) out vec2 vUV;
layout(location = 2) out vec3 vPosWS;
layout(location = 3) out vec3 vNormalWS;
layout(location = 4) out flat int vObjectIndex;   // per-instance object index into SSBO

layout(set = 0, binding = 0) uniform GlobalUBO {
    mat4 proj;
    mat4 view;
    vec3 camera_position;
} ubo;

// Per-instance/per-object data (indexed by objectIndex)
struct PerObjectData {
    mat4 model;
    uint material_index;
    int  entity_id;
};

// Storage buffer => std430 is the sane default for Vulkan SSBOs
layout(set = 2, binding = 0, std430) readonly buffer PerObjectSSBO {
    PerObjectData obj[];
};

void main()
{
    // Instance-local index + base instance (firstInstance from the draw call)
    uint objectIndex = gl_DrawIDARB;
    vObjectIndex = int(objectIndex);

    PerObjectData pd = obj[objectIndex];

    // World-space position
    vec4 posWS = pd.model * vec4(inPosition, 1.0);
    vPosWS = posWS.xyz;

    // World-space normal (correct for non-uniform scale)
    mat3 normalMat = mat3(transpose(inverse(pd.model)));
    vNormalWS = normalize(normalMat * inNormal);

    // Pass-through
    vColor = inColor;
    vUV    = inUV;

    // Clip-space
    gl_Position = ubo.proj * ubo.view * posWS;
}
