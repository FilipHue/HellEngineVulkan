#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inUV;
layout(location = 3) in vec3 inNormal;

layout(location = 0) out vec3 outColor;

layout(set = 0, binding = 0) uniform CameraUBO{
    mat4 view;
    mat4 proj;
} camera;

layout(set = 1, binding = 0) uniform ObjectUBO {
    mat4 model;
} object;

layout(push_constant) uniform PushConsts {
	vec4 color;
	vec4 position;
} push_consts;

void main() {
    outColor = inColor * push_consts.color.rgb;

	gl_Position = camera.proj * camera.view * (object.model * vec4(inPosition, 1.0) + push_consts.position);
}