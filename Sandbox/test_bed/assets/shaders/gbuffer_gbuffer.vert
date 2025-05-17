#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inUV;
layout(location = 3) in vec3 inNormal;

layout(set = 0, binding = 0) uniform CameraUBO{
    mat4 view;
    mat4 proj;
} camera;

layout(set = 1, binding = 0) uniform ObjectUBO {
    mat4 model;
} object;

layout (location = 0) out vec3 outNormal;
layout (location = 1) out vec3 outColor;
layout (location = 2) out vec3 outWorldPos;
layout (location = 3) out vec3 outTangent;

void main() {
	gl_Position = camera.proj * camera.view * object.model * vec4(inPosition, 1.0);

    // Vertex position in world space
	outWorldPos = vec3(object.model * vec4(inPosition, 1.0f));
	// GL to Vulkan coord space
	//outWorldPos.y = -outWorldPos.y;
	
	// Normal in world space
	mat3 mNormal = transpose(inverse(mat3(object.model)));
	outNormal = mNormal * normalize(inNormal);	
	
	// Currently just vertex color
	outColor = inColor;
}