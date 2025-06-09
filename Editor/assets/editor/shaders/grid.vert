#version 450

layout(set = 0, binding = 0) uniform CameraUBO {
    mat4 proj;
    mat4 view;
    vec3 pos;
} camera;

layout(location = 0) out float near;
layout(location = 1) out float far;
layout(location = 2) out vec3 near_point;
layout(location = 3) out vec3 far_point;
layout(location = 4) out mat4 view;
layout(location = 8) out mat4 proj;

vec3 gridPlane[6] = vec3[] (
    vec3(1, 1, 0), vec3(-1, -1, 0), vec3(-1, 1, 0),
    vec3(-1, -1, 0), vec3(1, 1, 0), vec3(1, -1, 0)
);

vec3 UnprojectPoint(float x, float y, float z, mat4 view, mat4 projection) {
    mat4 view_inv = inverse(view);
    mat4 proj_inv = inverse(projection);
    vec4 unprojected_point =  view_inv * proj_inv * vec4(x, y, z, 1.0);
    return unprojected_point.xyz / unprojected_point.w;
}

void main() {
    vec3 p = gridPlane[gl_VertexIndex].xyz;
    near_point = UnprojectPoint(p.x, p.y, 0.0, camera.view, camera.proj).xyz;
    far_point = UnprojectPoint(p.x, p.y, 1.0, camera.view, camera.proj).xyz;
    view = camera.view;
    proj = camera.proj;
    near = 0.1f;
    far = 1000.0f;
    gl_Position = vec4(p, 1.0);
}