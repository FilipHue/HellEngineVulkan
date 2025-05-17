#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec4 inColor;
layout(location = 2) in vec2 inUV;
layout(location = 3) in vec3 inNormal;

layout (location = 0) out vec3 outUV;

struct InstanceData
{
    mat4 model;
    float arrayIndex;
};

layout(set = 0, binding = 0) uniform CameraUBO {
    mat4 view;
    mat4 proj;

    InstanceData instanceData[8];
} ubo;

void main() {
	outUV = vec3(inUV, ubo.instanceData[gl_InstanceIndex].arrayIndex);

    mat4 model = ubo.instanceData[gl_InstanceIndex].model;
    gl_Position = ubo.proj * ubo.view * model * vec4(inPosition, 1.0);
}