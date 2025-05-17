#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inUV;
layout(location = 3) in vec3 inNormal;

layout (location = 0) out vec3 outNormal;
layout (location = 1) out vec3 outColor;
layout (location = 2) out vec3 outEyePos;
layout (location = 3) out vec3 outLightVec;

layout(set = 0, binding = 0) uniform CameraUBO{
    mat4 view;
    mat4 proj;
} camera;

layout(set = 1, binding = 0) uniform ObjectUBO {
    mat4 model;
} object;

layout(set = 2, binding = 0) uniform LightUBO{
    vec4 lightPos;
} light;

void main() 
{
	outNormal = inNormal;
	outColor = inColor;
	gl_Position = camera.proj * camera.view * object.model * vec4(inPosition, 1.0);
	outEyePos = vec3(camera.view * object.model * vec4(inPosition, 1.0));
	outLightVec = normalize(light.lightPos.xyz - outEyePos);

	// Clip against reflection plane
	vec4 clipPlane = vec4(0.0, -1.0, 0.0, 1.5);	
	gl_ClipDistance[0] = dot(vec4(inPosition, 1.0), clipPlane);	
}