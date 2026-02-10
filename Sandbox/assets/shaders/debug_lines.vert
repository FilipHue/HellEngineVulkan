#version 450

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec4 inColor;
layout(location = 2) in vec2 inUV;
layout(location = 3) in vec3 inNormal;

layout(set = 0, binding = 0) uniform CameraUBO
{
    mat4 view;
    mat4 projection;
} uCamera;

layout(location = 0) out vec4 vColor;

void main()
{
    gl_Position = uCamera.projection * uCamera.view * vec4(inPos, 1.0);
    vColor = inColor;
}
