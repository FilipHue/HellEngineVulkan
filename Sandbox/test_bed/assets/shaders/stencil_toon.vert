#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 3) in vec3 inNormal;

layout (location = 0) out vec3 outNormal;
layout (location = 1) out vec3 outColor;
layout (location = 2) out vec3 outLightVec;

layout(set = 0, binding = 0) uniform CameraUBO{
    mat4 view;
    mat4 projection;
} camera;

layout(set = 1, binding = 0) uniform ObjectUBO {
    mat4 model;
	vec4 lightPos;
} object;

out gl_PerVertex
{
	vec4 gl_Position;
};

void main() 
{
	outColor = vec3(1.0, 0.0, 0.0);
	mat4 modelView = camera.view * object.model;
	gl_Position = camera.projection * camera.view * object.model * vec4(inPosition, 1.0);
	outNormal = inNormal;
	vec4 pos = modelView * vec4(inPosition, 1.0);
	vec3 lPos = mat3(modelView) * object.lightPos.xyz;
	outLightVec = lPos - pos.xyz;
}