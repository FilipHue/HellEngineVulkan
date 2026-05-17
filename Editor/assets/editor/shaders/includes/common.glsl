// Common GLSL definitions and utility functions for the editor shaders
#ifndef COMMON_GLSL
#define COMMON_GLSL

// ================================
// Constants
// ================================

const float PI = 3.14159265359;
const float EPSILON = 0.00001;
const float INV_PI = 0.31830988618;

// ================================
// Structures
// ================================

struct CameraData {
	mat4 view;          // View matrix
	mat4 projection;    // Projection matrix
	vec3 position;      // Camera world position
};

struct WorldData {
    vec4 time;                      // x = total time, y = delta time
    vec4 ambient_color_intensity;   // xyz = color, w = intensity
};

struct PerObjectData {
    mat4 model;             // Model matrix
    uint material_index;    // Index into material array
    int  entity_id;         // Entity ID for picking
};

struct MaterialInfo {
    int diffuse;
    int specular;
    int ambient;
    int emissive;
    int height;
    int normal;
    int shininess;
    int opacity;
    int displacement;
    int lightmap;
    int reflection;

    int base_color;
    int normal_camera;
    int emission_color;
    int metalness;
    int diffuse_roughness;
    int ambient_occlusion;
    int sheen;
    int clearcoat;
    int transmission;
};

// ================================
// Utility Functions
// ================================

// Generate pseudo-random number based on position
float Random(vec2 co)
{
    return fract(sin(dot(co.xy, vec2(12.9898, 78.233))) * 43758.5453);
}

// Generate random direction in hemisphere around normal
vec3 RandomHemisphereDirection(vec3 normal, vec2 seed)
{
    float u = Random(seed);
    float v = Random(seed + vec2(1.0, 0.0));

    float phi = 2.0 * PI * u;
    float cosTheta = sqrt(1.0 - v);
    float sinTheta = sqrt(v);

    vec3 tangent = normalize(cross(normal, abs(normal.y) > 0.9 ? vec3(1.0, 0.0, 0.0) : vec3(0.0, 1.0, 0.0)));
    vec3 bitangent = cross(normal, tangent);

    return normalize(
        tangent * (sinTheta * cos(phi)) +
        bitangent * (sinTheta * sin(phi)) +
        normal * cosTheta
    );
}

// ================================
// Color Space Conversions
// ================================

// Linear to sRGB
vec3 LinearToSRGB(vec3 color)
{
	return pow(color, vec3(1.0 / 2.2));
}

// sRGB to Linear
vec3 SRGBToLinear(vec3 color)
{
	return pow(color, vec3(2.2));
}

// Luminance calculation
float Luminance(vec3 color)
{
	return dot(color, vec3(0.2126, 0.7152, 0.0722));
}

// Unpack normal from [0,1] to [-1,1]
vec3 UnpackNormal(vec3 normal)
{
	return normalize(normal * 2.0 - 1.0);
}

// Calculate F0 (base reflectivity)
vec3 CalculateF0(vec3 albedo, float metallic)
{
	vec3 F0 = vec3(0.04); // Dielectric base reflectivity
	return mix(F0, albedo, metallic);
}

// TBN matrix construction
mat3 CalculateTBN(vec3 N, vec3 T, vec3 B)
{
	T = normalize(T - dot(T, N) * N); // Gram-Schmidt orthogonalization
	B = cross(N, T);
	return mat3(T, B, N);
}

// Sample normal map and transform to world space
vec3 SampleNormalMap(sampler2D normalMap, vec2 uv, mat3 TBN)
{
	vec3 normal = texture(normalMap, uv).rgb;
	normal = UnpackNormal(normal);
	return normalize(TBN * normal);
}

#endif // COMMON_GLSL
