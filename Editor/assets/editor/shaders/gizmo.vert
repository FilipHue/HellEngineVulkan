#version 460

// Input vertex attributes
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec4 inColor;

// Output to the fragment shader
layout(location = 0) out vec4 vColor;

// Push constants for the model-view-projection matrix
layout(push_constant) uniform GizmoData {
	mat4 mvp;
} pc_gizmoData;

void main()
{
	vColor = inColor;

	gl_Position = pc_gizmoData.mvp * vec4(inPosition, 1.0);
}
