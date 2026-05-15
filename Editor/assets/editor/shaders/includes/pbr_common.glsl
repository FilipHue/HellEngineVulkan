#ifndef PBR_COMMON_GLSL
#define PBR_COMMON_GLSL

// ================================
// Distribution Functions
// ================================

// Normal Distribution Function (GGX/Trowbridge-Reitz)
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float nom = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / max(denom, EPSILON);
}

// Beckmann Distribution (alternative to GGX)
float DistributionBeckmann(vec3 N, vec3 H, float roughness)
{
	float a = roughness * roughness;
	float a2 = a * a;
	float NdotH = max(dot(N, H), 0.0);
	float NdotH2 = NdotH * NdotH;

	float nom = exp((NdotH2 - 1.0) / (a2 * NdotH2));
	float denom = PI * a2 * NdotH2 * NdotH2;

	return nom / max(denom, EPSILON);
}

// ================================
// Geometry Functions
// ================================

// Geometry Function (Smith's Schlick-GGX)
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float nom = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / max(denom, EPSILON);
}

// Schlick-GGX for IBL
float GeometrySchlickGGX_IBL(float NdotV, float roughness)
{
	float a = roughness;
	float k = (a * a) / 2.0;

	float nom = NdotV;
	float denom = NdotV * (1.0 - k) + k;

	return nom / max(denom, EPSILON);
}

// Smith's method - combines view and light geometry occlusion
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

// Smith's method for IBL
float GeometrySmith_IBL(vec3 N, vec3 V, vec3 L, float roughness)
{
	float NdotV = max(dot(N, V), 0.0);
	float NdotL = max(dot(N, L), 0.0);
	float ggx2 = GeometrySchlickGGX_IBL(NdotV, roughness);
	float ggx1 = GeometrySchlickGGX_IBL(NdotL, roughness);

	return ggx1 * ggx2;
}

// ================================
// Fresnel Functions
// ================================

// Fresnel-Schlick Approximation
vec3 FresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

// Fresnel with roughness for IBL
vec3 FresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

// Exact Fresnel (Cook-Torrance)
vec3 FresnelCookTorrance(float cosTheta, vec3 F0)
{
	vec3 eta = (1.0 + sqrt(F0)) / (1.0 - sqrt(F0));
	vec3 g = sqrt(eta * eta + cosTheta * cosTheta - 1.0);

	vec3 gMinusC = g - cosTheta;
	vec3 gPlusC = g + cosTheta;

	vec3 part1 = (gMinusC * gMinusC) / (gPlusC * gPlusC);
	vec3 part2 = 1.0 + pow(((cosTheta * gPlusC - 1.0) / (cosTheta * gMinusC + 1.0)), vec3(2.0));

	return 0.5 * part1 * part2;
}

// ================================
// Cook-Torrance BRDF
// ================================

vec3 CookTorranceBRDF(vec3 N, vec3 V, vec3 L, vec3 albedo, float metallic, float roughness, vec3 F0)
{
	vec3 H = normalize(V + L);

	// Calculate BRDF components
	float NDF = DistributionGGX(N, H, roughness);
	float G = GeometrySmith(N, V, L, roughness);
	vec3 F = FresnelSchlick(max(dot(H, V), 0.0), F0);

	// Specular contribution
	vec3 numerator = NDF * G * F;
	float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0);
	vec3 specular = numerator / max(denominator, EPSILON);

	// Energy conservation
	vec3 kS = F;
	vec3 kD = vec3(1.0) - kS;
	kD *= 1.0 - metallic;

	// Lambert diffuse
	vec3 diffuse = kD * albedo * INV_PI;

	return diffuse + specular;
}

// ================================
// Tone Mapping Functions
// ================================

// ACES Tone Mapping
vec3 ACESFilm(vec3 x)
{
    float a = 2.51;
    float b = 0.03;
    float c = 2.43;
    float d = 0.59;
    float e = 0.14;
    return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}

// Reinhard Tone Mapping
vec3 Reinhard(vec3 color)
{
	return color / (color + vec3(1.0));
}

// Uncharted 2 Tone Mapping
vec3 Uncharted2Tonemap(vec3 x)
{
	float A = 0.15;
	float B = 0.50;
	float C = 0.10;
	float D = 0.20;
	float E = 0.02;
	float F = 0.30;
	return ((x * (A * x + C * B) + D * E) / (x * (A * x + B) + D * F)) - E / F;
}

vec3 Uncharted2(vec3 color)
{
	float exposureBias = 2.0;
	vec3 curr = Uncharted2Tonemap(color * exposureBias);
	vec3 W = vec3(11.2);
	vec3 whiteScale = vec3(1.0) / Uncharted2Tonemap(W);
	return curr * whiteScale;
}

#endif // PBR_COMMON_GLSL
