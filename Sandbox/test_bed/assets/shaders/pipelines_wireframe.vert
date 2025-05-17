#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inUV;
layout(location = 3) in vec3 inNormal;

layout(location = 0) out vec3 outColor;
layout(location = 1) out vec2 outUV;

layout(set = 0, binding = 0) uniform CameraUBO {
    mat4 view;
    mat4 proj;
} camera;

layout(set = 1, binding = 0) uniform ObjectUBO {
    mat4 model;
} object;

void main() {
    outColor = inColor;
    outUV = inUV;

	gl_Position = camera.proj * camera.view * object.model * vec4(inPosition, 1.0);
}