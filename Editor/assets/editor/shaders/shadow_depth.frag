#version 460

layout(location = 0) in vec3 vWorldPos;

layout(push_constant) uniform PushConstants
{
    mat4  lightViewProj;
    vec4  lightPosAndFar;  // xyz = light world pos, w = far plane (0 = not point light)
} pc;

void main()
{
    if (pc.lightPosAndFar.w > 0.0)
    {
        // Point light — write linear depth normalized by far plane
        float lightDistance = length(vWorldPos - pc.lightPosAndFar.xyz);
        gl_FragDepth = lightDistance / pc.lightPosAndFar.w;
    }
    else
    {
        gl_FragDepth = gl_FragCoord.z;  // Directional light — use regular depth
    }
}