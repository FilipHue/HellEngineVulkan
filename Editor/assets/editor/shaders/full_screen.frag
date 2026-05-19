// ============================
// Full-screen Fragment Shader
// ============================
#version 460

// Vertex attributes
layout (location = 0) in vec2 inUV;

// Inputs
layout(set = 0, binding = 0) uniform sampler2D renderTarget;

// Outputs
layout (location = 0) out vec4 outFragColor;

void main() 
{
	outFragColor = texture(renderTarget, inUV);
}
