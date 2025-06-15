#version 460
#extension GL_EXT_nonuniform_qualifier : enable

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

layout (set = 1, binding = 0, std140) readonly buffer MaterialUBO {
    MaterialInfo info[];
};

layout (set = 2, binding = 0) uniform sampler2D textures[];

layout (set = 0, binding = 0) uniform GlobalUBO {
    mat4 proj;
    mat4 view;
    vec3 camera_position;
} ubo;

layout (push_constant) uniform PushConstants {
    uint material_index;
} push_constants;

layout (location = 0) in vec3 fragColor;
layout (location = 1) in vec2 fragUV;
layout (location = 2) in vec3 fragPosition;
layout (location = 3) in vec3 fragNormal;

layout (location = 0) out vec4 outFragColor;
layout (location = 1) out uint outPickId;

// Temporary: a single white light for now
const vec3 lightPos = vec3(0.0, 10.0, 0.0);
const vec3 lightColor = vec3(1.0, 1.0, 1.0);
const float ambientStrength = 0.1;
const float specularStrength = 0.5;

void main() {
    vec3 norm = normalize(fragNormal);
    vec3 lightDir = normalize(lightPos - fragPosition);
    vec3 viewDir = normalize(ubo.camera_position - fragPosition);
    vec3 reflectDir = reflect(-lightDir, norm);

    // Diffuse
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    // Specular
    float shininess = 32.0;

    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    vec3 specular = specularStrength * spec * lightColor;

    // Ambient
    vec3 ambient = ambientStrength * lightColor;

    vec4 texColor = texture(textures[info[push_constants.material_index].diffuse], fragUV);
    vec3 finalColor = (ambient + diffuse + specular) * texColor.rgb;

    if (texColor.a < 0.1) {
        discard;
    }

    outFragColor = vec4(finalColor, texColor.a);
    outPickId = int(push_constants.material_index + 1);
}
