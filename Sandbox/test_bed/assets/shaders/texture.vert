#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec4 inColor;
layout(location = 2) in vec2 inUV;
layout(location = 3) in vec3 inNormal;

layout (location = 0) out vec2 outUV;
layout (location = 1) out float outLodBias;
layout (location = 2) out vec3 outNormal;
layout (location = 3) out vec3 outViewVec;
layout (location = 4) out vec3 outLightVec;

layout(set = 0, binding = 0) uniform CameraUBO {
    mat4 view;
    mat4 proj;
} camera;

layout(set = 1, binding = 0) uniform ObjectUBO {
    mat4 model;
} object;

layout(set = 2, binding = 0) uniform LightUBO {
    vec4 viewPos;
} light;

layout (set = 3, binding = 0) uniform ImageUBO {
	float lod;
} image;

out gl_PerVertex 
{
    vec4 gl_Position;   
};

void main() {
	outUV = inUV;
	outLodBias = image.lod;

	vec3 worldPos = vec3(object.model * vec4(inPosition, 1.0));

	gl_Position = camera.proj * camera.view * object.model * vec4(inPosition.xyz, 1.0);

    vec4 pos = object.model * vec4(inPosition, 1.0);
	outNormal = mat3(inverse(transpose(object.model))) * inNormal;
	vec3 lightPos = vec3(0.0);
	vec3 lPos = mat3(object.model) * lightPos.xyz;
    outLightVec = lPos - pos.xyz;
    outViewVec = light.viewPos.xyz - pos.xyz;		
}