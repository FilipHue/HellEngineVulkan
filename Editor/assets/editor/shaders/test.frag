#version 460
#extension GL_EXT_nonuniform_qualifier   : enable

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

struct PerDrawData {
    mat4 model;
    uint material_index;
    int  entity_id;
};

layout(set = 0, binding = 0) uniform GlobalUBO {
    mat4 proj;
    mat4 view;
    vec3 camera_position;
} ubo;

layout(set = 1, binding = 0, std140) readonly buffer MaterialSSBO {
    MaterialInfo info[];
};

layout(set = 2, binding = 0, std140) readonly buffer PerDrawSSBO {
    PerDrawData draw_info[];
};

layout(set = 3, binding = 0) uniform sampler2D textures[];

layout(location = 0) in  vec3 vColor;
layout(location = 1) in  vec2 vUV;
layout(location = 2) in  vec3 vPosWS;
layout(location = 3) in  vec3 vNormalWS;
layout(location = 4) in  flat int drawID;

layout(location = 0) out vec4 outFragColor;
layout(location = 1) out uint outPickId;

const vec3  kLightPos        = vec3( 0.0, 10.0,  0.0);
const vec3  kLightColor      = vec3( 1.0,  1.0,  1.0);
const float kAmbientStrength = 0.1;
const float kSpecularStrength= 0.5;

void main()
{
    PerDrawData pd     = draw_info[drawID];
    MaterialInfo mat   = info[pd.material_index];

    vec4 texColor = texture( textures[mat.diffuse], vUV );
    if (texColor.a < 0.1) { discard; }

    vec3  N      = normalize(vNormalWS);
    vec3  L      = normalize(kLightPos - vPosWS);
    vec3  V      = normalize(ubo.camera_position - vPosWS);
    vec3  R      = reflect(-L, N);

    float diff   = max(dot(N, L), 0.0);
    float shin   = (mat.shininess > 0) ? float(mat.shininess) : 32.0;
    float spec   = pow(max(dot(V, R), 0.0), shin);

    vec3 ambient  = kAmbientStrength * kLightColor;
    vec3 diffuse  = diff               * kLightColor;
    vec3 specular = kSpecularStrength  * spec * kLightColor;

    vec3 finalRGB = (ambient + diffuse + specular) * texColor.rgb;

    outFragColor = vec4(finalRGB, texColor.a);
    outPickId    = uint(pd.entity_id);
}
