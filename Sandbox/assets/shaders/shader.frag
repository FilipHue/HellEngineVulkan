#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

layout(set = 2, binding = 2) uniform sampler2D texSampler1;
layout(set = 2, binding = 3) uniform sampler2D texSampler2;

void main() {
   vec4 color1 = texture(texSampler1, fragTexCoord);
   vec4 color2 = texture(texSampler2, fragTexCoord);

   outColor = mix(color1, color2, 0.5f);
}