#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inUV;
layout(location = 3) in vec3 inNormal;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragUV;
layout(location = 2) out vec3 fragPosition;
layout(location = 3) out vec3 fragNormal;

layout(set = 0, binding = 0) uniform GlobalUBO {
    mat4 proj;
    mat4 view;
    vec3 camera_position;
} ubo;

void main() {
    fragPosition = inPosition;
    fragNormal = inNormal;
    fragColor = inColor;
    fragUV = inUV;

    gl_Position = ubo.proj * ubo.view * vec4(inPosition, 1.0);
}
