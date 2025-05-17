#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inUV;
layout(location = 3) in vec3 inNormal;

layout (location = 0) out vec3 outColor;
layout (location = 1) out vec3 outNormal;
layout (location = 2) out vec3 outViewVec;
layout (location = 3) out vec3 outLightVec;

layout(set = 0, binding = 0) uniform CameraUBO{
    mat4 view;
    mat4 proj;
} camera;

layout(set = 1, binding = 0) uniform ObjectUBO {
    mat4 model;
} object;

void main() {
	gl_Position = camera.proj * camera.view * object.model * vec4(inPosition, 1.0);

    outColor = inColor;
	outNormal = inNormal;
	outLightVec = vec3(0.0f, -5.0f, 15.0f) - inPosition;
	outViewVec = -inPosition.xyz;	
}