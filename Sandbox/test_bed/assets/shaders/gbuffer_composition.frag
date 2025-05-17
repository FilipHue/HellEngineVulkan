#version 450

layout (set = 0, binding = 0) uniform sampler2D inputPosition;
layout (set = 0, binding = 1) uniform sampler2D inputNormal;
layout (set = 0, binding = 2) uniform sampler2D inputAlbedo;

struct Light {
	vec4 position;
	vec3 color;
	float radius;
};

layout (set = 0, std140, binding = 3) buffer LightsBuffer 
{
	Light lights[];
};

layout (location = 0) in vec2 inUV;

layout (location = 0) out vec4 outColor;

void main() 
{
	// Read G-Buffer values from previous sub pass
	vec3 fragPos = texture(inputPosition, inUV).rgb;
	vec3 normal = texture(inputNormal, inUV).rgb;
	vec4 albedo = texture(inputAlbedo, inUV);
	
	#define ambient 0.05
	
	// Ambient part
	vec3 fragcolor  = albedo.rgb * ambient;
	
	for(int i = 0; i < lights.length(); ++i)
	{
		vec3 L = lights[i].position.xyz - fragPos;
		float dist = length(L);

		L = normalize(L);
		float atten = lights[i].radius / (pow(dist, 3.0) + 1.0);

		vec3 N = normalize(normal);
		float NdotL = max(0.0, dot(N, L));
		vec3 diff = lights[i].color * albedo.rgb * NdotL * atten;

		fragcolor += diff;
	}    	
	outColor = vec4(fragcolor, 1.0);
}