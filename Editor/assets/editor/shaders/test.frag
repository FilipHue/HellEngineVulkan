// ================================
// Vulkan GLSL Fragment Shader (instanced + bindless textures)
// ================================
#version 460

// Needed when indexing a descriptor array with a value that may vary between lanes
#extension GL_EXT_nonuniform_qualifier : enable

layout(location = 0) in vec3 vColor;
layout(location = 1) in vec2 vUV;
layout(location = 2) in vec3 vPosWS;
layout(location = 3) in vec3 vNormalWS;
layout(location = 4) in flat int vObjectIndex;

layout(location = 0) out vec4 outFragColor;
layout(location = 1) out uint outPickId;

// Keep UBOs as std430 (default for uniform blocks)
layout(set = 0, binding = 0) uniform GlobalUBO {
    mat4 proj;
    mat4 view;
    vec3 camera_position;
} ubo;

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

struct PerObjectData {
    mat4 model;
    uint material_index;
    int  entity_id;
};

// SSBOs as std430
layout(set = 1, binding = 0, std430) readonly buffer MaterialSSBO {
    MaterialInfo info[];
};

layout(set = 2, binding = 0, std430) readonly buffer PerObjectSSBO {
    PerObjectData obj[];
};

// Bindless-style texture array (requires correct descriptor indexing features on Vulkan side)
layout(set = 3, binding = 0) uniform sampler2D textures[];

// Lighting constants (toy model; replace with your own light system)
const vec3  kLightPos         = vec3(0.0, 10.0, 0.0);
const vec3  kLightColor       = vec3(1.0);
const float kAmbientStrength  = 0.10;
const float kSpecularStrength = 0.50;

void main()
{
    PerObjectData pd   = obj[uint(vObjectIndex)];
    MaterialInfo  mat  = info[pd.material_index];

    // Sample diffuse texture (non-uniform indexing is important here)
    uint texIndex = uint(max(mat.diffuse, 0)); // basic safety clamp if you ever store -1
    vec4 texColor = texture(textures[nonuniformEXT(texIndex)], vUV);

    // Alpha test (optional)
    if (texColor.a < 0.1) discard;

    vec3 N = normalize(vNormalWS);
    vec3 L = normalize(kLightPos - vPosWS);
    vec3 V = normalize(ubo.camera_position - vPosWS);
    vec3 R = reflect(-L, N);

    float diff = max(dot(N, L), 0.0);

    float shin = (mat.shininess > 0) ? float(mat.shininess) : 32.0;
    float spec = pow(max(dot(V, R), 0.0), shin);

    vec3 ambient  = kAmbientStrength  * kLightColor;
    vec3 diffuse  = diff              * kLightColor;
    vec3 specular = kSpecularStrength * spec * kLightColor;

    vec3 lit = (ambient + diffuse + specular) * texColor.rgb;

    outFragColor = vec4(lit, texColor.a);
    outPickId    = uint(pd.entity_id);
}
