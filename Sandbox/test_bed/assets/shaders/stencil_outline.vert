#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 3) in vec3 inNormal;

layout(set = 0, binding = 0) uniform CameraUBO{
    mat4 view;
    mat4 projection;
} camera;

layout(set = 1, binding = 0) uniform ObjectUBO {
    mat4 model;
	float outlineWidth;
} object;

out gl_PerVertex
{
	vec4 gl_Position;
};

void main() 
{
	// Extrude along normal
	vec3 pos = inPosition + inNormal * object.outlineWidth;
	gl_Position = camera.projection * camera.view * object.model * vec4(pos, 1.0f);
}