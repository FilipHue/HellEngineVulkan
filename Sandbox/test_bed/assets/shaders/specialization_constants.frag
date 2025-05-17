#version 450

layout (set = 3, binding = 0) uniform MaterialUBO {
	vec4 color;
} material;
layout (set = 4, binding = 0) uniform sampler2D samplerColormap;

layout (location = 0) in vec3 inNormal;
layout (location = 1) in vec3 inColor;
layout (location = 2) in vec2 inUV;
layout (location = 3) in vec3 inViewVec;
layout (location = 4) in vec3 inLightVec;

layout (location = 0) out vec4 outFragColor;

// We use this constant to control the flow of the shader depending on the 
// lighting model selected at pipeline creation time
layout (constant_id = 0) const int LIGHTING_MODEL = 0;
// Parameter for the toon shading part of the shader
layout (constant_id = 1) const float PARAM_TOON_DESATURATION = 0.0f;
/*
Bubble Sort:
bool sortat = false;
while (!sortat)
{
	sortat = true;
	for (int i = 0; i < n - 1; i++)
	{
		if (v[i] > v[i + 1])
		{
			swap(v[i], v[i + 1]);
			sortat = false;
		}
	}
}
*/


void main() 
{
	switch (LIGHTING_MODEL) {
		case 0: // Phong			
		{
			vec3 ambient = material.color.xyz * vec3(0.25);
			vec3 N = normalize(inNormal);
			vec3 L = normalize(inLightVec);
			vec3 V = normalize(inViewVec);
			vec3 R = reflect(-L, N);
			vec3 diffuse = max(dot(N, L), 0.0) * material.color.xyz;
			vec3 specular = pow(max(dot(R, V), 0.0), 32.0) * vec3(0.75);
			outFragColor = vec4(ambient + diffuse * 1.75 + specular, 1.0);
			//outFragColor = vec4(1.0, 0.0, 0.0, 1.0);
			break;
		}
		case 1: // Toon
		{

			vec3 N = normalize(inNormal);
			vec3 L = normalize(inLightVec);
			float intensity = dot(N,L);
			vec3 color;
			if (intensity > 0.98)
				color = material.color.xyz * 1.5;
			else if  (intensity > 0.9)
				color = material.color.xyz * 1.0;
			else if (intensity > 0.5)
				color = material.color.xyz * 0.6;
			else if (intensity > 0.25)
				color = material.color.xyz * 0.4;
			else
				color = material.color.xyz * 0.2;
			// Desaturate a bit
			color = vec3(mix(color, vec3(dot(vec3(0.2126,0.7152,0.0722), color)), PARAM_TOON_DESATURATION));	
			outFragColor.rgb = color;
			//outFragColor = vec4(0.0, 1.0, 0.0, 1.0);
			break;
		}
		case 2: // Textured
		{
			vec4 color = texture(samplerColormap, inUV).rrra;
			vec3 ambient = color.rgb * vec3(0.25) * material.color.xyz;
			vec3 N = normalize(inNormal);
			vec3 L = normalize(inLightVec);
			vec3 V = normalize(inViewVec);
			vec3 R = reflect(-L, N);
			vec3 diffuse = max(dot(N, L), 0.0) * color.rgb;
			float specular = pow(max(dot(R, V), 0.0), 32.0) * color.a;
			outFragColor = vec4(ambient + diffuse + vec3(specular), 1.0);
			//outFragColor = vec4(0.0, 0.0, 1.0, 1.0);
			break;
		}
	}
}