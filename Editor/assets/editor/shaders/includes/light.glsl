// Light calculations for PBR shading
#ifndef LIGHT_GLSL
#define LIGHT_GLSL

#include "pbr_common.glsl"

// ================================
// Defines
// ================================

#define MAX_LIGHTS 16
#define MAX_SHADOW_CASCADES 4
#define MAX_SHADOW_MAPS (MAX_LIGHTS * MAX_SHADOW_CASCADES)  // 2D shadow maps (directional + spot)
#define MAX_POINT_SHADOW_MAPS MAX_LIGHTS                    // One cubemap per point light

// ================================
// Structures
// ================================

struct LightInfo {
    vec4 position_type;      // xyz = position or direction, w = type
    vec4 direction_range;    // xyz = direction, w = range
    vec4 color_intensity;    // xyz = color, w = intensity
    vec4 cone_attenuation;   // x = inner, y = outer, z = attenuation, w = enabled
    vec4 shadow_params;      // x = base shadow map index, y = bias, z = cascade count/strength, w = cast_shadows

    mat4 shadow_matrix;      // spot light shadow matrix

    mat4 cascade_matrices[MAX_SHADOW_CASCADES];
    vec4 cascade_splits;

    mat4 point_matrices[6];  // point light: one matrix per cube face
};

struct PointLight {
	vec3 position;
	vec3 color;
	float intensity;
	float radius;
};

struct DirectionalLight {
	vec3 position;
	vec3 color;
	float intensity;
	vec3 direction;
};

struct SpotLight {
	vec3 position;
	vec3 color;
	float intensity;
	vec3 direction;
	float innerCutoff;
	float outerCutoff;
};

// ================================
// Functions
// ================================

// Point light contribution
vec3 CalculatePointLight(PointLight light, vec3 N, vec3 V, vec3 fragPos, vec3 albedo, float metallic, float roughness, vec3 F0)
{
	vec3 L = normalize(light.position - fragPos);
	float distance = length(light.position - fragPos);

	// Attenuation with smooth falloff
	float attenuation = light.intensity / (distance * distance);
	float falloff = max(1.0 - pow(distance / light.radius, 4.0), 0.0);
	attenuation *= falloff * falloff;

	vec3 radiance = light.color * attenuation;

	// BRDF
	vec3 brdf = CookTorranceBRDF(N, V, L, albedo, metallic, roughness, F0);

	// Outgoing radiance
	float NdotL = max(dot(N, L), 0.0);
	return brdf * radiance * NdotL;
}

// Directional light contribution
vec3 CalculateDirectionalLight(DirectionalLight light, vec3 N, vec3 V, vec3 albedo, float metallic, float roughness, vec3 F0)
{
	vec3 L = normalize(-light.direction);
	vec3 radiance = light.color * light.intensity;

	// BRDF
	vec3 brdf = CookTorranceBRDF(N, V, L, albedo, metallic, roughness, F0);

	// Outgoing radiance
	float NdotL = max(dot(N, L), 0.0);
	return brdf * radiance * NdotL;
}

// Spot light contribution
vec3 CalculateSpotLight(SpotLight light, vec3 N, vec3 V, vec3 fragPos, vec3 albedo, float metallic, float roughness, vec3 F0)
{
	vec3 L = normalize(light.position - fragPos);
	float distance = length(light.position - fragPos);

	// Attenuation
	float attenuation = light.intensity / (distance * distance);

	// Spotlight intensity (smooth edges)
	float theta = dot(L, normalize(-light.direction));
	float epsilon = light.innerCutoff - light.outerCutoff;
	float intensity = clamp((theta - light.outerCutoff) / epsilon, 0.0, 1.0);

	vec3 radiance = light.color * attenuation * intensity;

	// BRDF
	vec3 brdf = CookTorranceBRDF(N, V, L, albedo, metallic, roughness, F0);

	// Outgoing radiance
	float NdotL = max(dot(N, L), 0.0);
	return brdf * radiance * NdotL;
}

#endif // LIGHT_GLSL
