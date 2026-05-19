// ======================
// Gizmo Fragment Shader
// ======================
#version 460

// Inputs
layout(location = 0) in vec4 vColor;

// Outputs
layout(location = 0) out vec4 outColor;

void main()
{
	outColor = vColor;
}
