#version 450
#extension GL_EXT_nonuniform_qualifier : enable

struct MaterialInfo
{
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

layout (set = 1, binding = 0, std140) readonly buffer MaterialUBO
{
	MaterialInfo info[];
};
layout (set = 2, binding = 0) uniform sampler2D textures[];

layout(push_constant) uniform PushConstants {
    uint material_index;
} push_constants;

layout (location = 0) in vec3 inColor;
layout (location = 1) in vec2 inUV;

layout (location = 0) out vec4 outFragColor;

void main() 
{
	vec4 color = texture(textures[info[push_constants.material_index].diffuse], inUV);

	outFragColor = color;
}