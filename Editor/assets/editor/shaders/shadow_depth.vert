#version 460
#extension GL_ARB_shader_draw_parameters : enable

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inUV;
layout(location = 3) in vec3 inNormal;
layout(location = 4) in vec3 inTangent;
layout(location = 5) in vec3 inBitangent;

struct PerObjectData {
    mat4 model;
    uint material_index;
    int  entity_id;
};

layout(set = 2, binding = 0, std430) readonly buffer PerObjectSSBO
{
    PerObjectData obj[];
} perObject;

layout(push_constant) uniform PushConstants
{
    mat4  lightViewProj;
    vec4  lightPosAndFar;  // xyz = light world pos, w = far plane (0 = not point light)
} pc;

layout(location = 0) out vec3 vWorldPos;

void main()
{
    uint objectIndex = gl_DrawIDARB;
    mat4 model       = perObject.obj[objectIndex].model;
    vec4 worldPos    = model * vec4(inPosition, 1.0);
    vWorldPos        = worldPos.xyz;
    gl_Position      = pc.lightViewProj * worldPos;
}