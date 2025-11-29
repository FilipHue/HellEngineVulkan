#version 460

#extension GL_ARB_shader_draw_parameters : require

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inUV;
layout(location = 3) in vec3 inNormal;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragUV;
layout(location = 2) out vec3 fragPosition;
layout(location = 3) out vec3 fragNormal;
layout(location = 4) out int fragDrawID;

layout(set = 0, binding = 0) uniform GlobalUBO {
    mat4 proj;
    mat4 view;
    vec3 camera_position;
} ubo;

// layout (push_constant) uniform PushConstants {
//     mat4 model;
//     uint material_index;
// } push_constants;

struct PerDrawData {
    mat4 model;
    uint material_index;
    int  entity_id;
};

layout(set = 2, binding = 0, std430) readonly buffer PerDrawSSBO {
    PerDrawData draw_info[];
};

void main() {
    fragPosition = inPosition;
    fragNormal = inNormal;
    fragColor = inColor;
    fragUV = inUV;
    fragDrawID = gl_DrawIDARB;

    gl_Position = ubo.proj * ubo.view * draw_info[fragDrawID].model * vec4(inPosition, 1.0);
}
