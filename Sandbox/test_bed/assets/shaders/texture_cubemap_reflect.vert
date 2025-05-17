#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inUV;
layout(location = 3) in vec3 inNormal;

layout(location = 0) out vec3 outPos;
layout(location = 1) out vec3 outNormal;
layout(location = 2) out vec3 outViewVec;
layout(location = 3) out vec3 outLightVec;

layout(binding = 0) uniform UBO
{
	mat4 projection;
	mat4 model;
	mat4 invModel;
	float lodBias;
} ubo;

void main()
{
	gl_Position = ubo.projection * ubo.model * vec4(inPosition.xyz, 1.0);

	outPos = vec3(ubo.model * vec4(inPosition, 1.0));
	outNormal = mat3(ubo.model) * inNormal;

	vec3 lightPos = vec3(0.0f, 5.0f, 5.0f);
	outLightVec = lightPos.xyz - outPos.xyz;
	outViewVec = -outPos.xyz;
}