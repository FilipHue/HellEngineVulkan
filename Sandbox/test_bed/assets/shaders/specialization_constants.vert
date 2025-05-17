#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inUV;
layout(location = 3) in vec3 inNormal;

layout (location = 0) out vec3 outNormal;
layout (location = 1) out vec3 outColor;
layout (location = 2) out vec2 outUV;
layout (location = 3) out vec3 outViewVec;
layout (location = 4) out vec3 outLightVec;

layout(set = 0, binding = 0) uniform CameraUBO{
    mat4 view;
    mat4 proj;
} camera;

layout(set = 1, binding = 0) uniform ObjectUBO {
    mat4 model;
} object;

layout(set = 2, binding = 0) uniform LightUBO{
    vec4 lightPos;
    vec4 viewPos;
} light;

out gl_PerVertex
{
	vec4 gl_Position;
};

void main() {
	outColor = inColor;
	outUV = inUV;
	outNormal = inNormal;

	gl_Position = camera.proj * camera.view * object.model * vec4(inPosition, 1.0);
	
	vec4 pos = camera.view * vec4(inPosition, 1.0);
	outNormal = mat3(camera.view) * inNormal;

	vec3 lPos = mat3(camera.view) * light.lightPos.xyz;
	outLightVec = light.lightPos.xyz - pos.xyz;
	outViewVec = light.viewPos.xyz - pos.xyz;	
}