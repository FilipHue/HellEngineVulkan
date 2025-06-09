#version 450

layout(location = 0) in float near;
layout(location = 1) in float far;
layout(location = 2) in vec3 near_point;
layout(location = 3) in vec3 far_point;
layout(location = 4) in mat4 view;
layout(location = 8) in mat4 proj;
layout(location = 0) out vec4 outColor;

vec4 grid(vec3 pos, float scale, bool drawAxis) {
    vec2 coord = pos.xz * scale;
    vec2 derivative = fwidth(coord);
    vec2 grid = abs(fract(coord - 0.5) - 0.5) / derivative;
    float line = min(grid.x, grid.y);
    float minimumz = min(derivative.y, 1);
    float minimumx = min(derivative.x, 1);
    vec4 color = vec4(0.2, 0.2, 0.2, 1.0 - min(line, 1.0));
    // z axis
    if(pos.x > -0.1 * minimumx && pos.x < 0.1 * minimumx)
        color.z = 1.0;
    // x axis
    if(pos.z > -0.1 * minimumz && pos.z < 0.1 * minimumz)
        color.x = 1.0;
    return color;
}
float ComputeDepth(vec3 pos) {
    vec4 clip_space_pos = proj * view * vec4(pos.xyz, 1.0);
    return (clip_space_pos.z / clip_space_pos.w);
}
float ComputeLinearDepth(vec3 pos) {
    vec4 clip_space_pos = proj * view * vec4(pos.xyz, 1.0);
    float clip_space_depth = (clip_space_pos.z / clip_space_pos.w) * 2.0 - 1.0;
    float linearDepth = (2.0 * near * far) / (far + near - clip_space_depth * (far - near));
    return linearDepth / far;
}
void main() {
    float t = -near_point.y / (far_point.y - near_point.y);
    vec3 pos = near_point + t * (far_point - near_point);

    gl_FragDepth = ComputeDepth(pos);

    float linearDepth = ComputeLinearDepth(pos);
    float fading = max(0, (0.5 - linearDepth));

    outColor = (grid(pos, 10, true) + grid(pos, 1, true)) * float(t > 0);
    outColor.a *= fading;

    if (outColor.a == 0.0f)
    {
        discard;
    }
}