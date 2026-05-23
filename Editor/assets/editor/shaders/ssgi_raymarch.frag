#version 460

layout(location = 0) in vec2 vUV;
layout(location = 0) out vec4 oGI;

// set=0 matches your pipeline layout for SSGI raymarch
layout(set = 0, binding = 0) uniform sampler2D uDepth;
layout(set = 0, binding = 1) uniform sampler2D uSceneColor;

// Must match your GlobalShaderData (add inv_view_proj for best performance)
layout(set = 0, binding = 2) uniform GlobalUBO
{
    mat4 proj;
    mat4 view;
    vec3 camera_position;
    float _pad0;

    mat4 inv_view_proj;   // add this in C++ (recommended)
} ubo;

float Hash12(vec2 p)
{
    vec3 p3 = fract(vec3(p.xyx) * 0.1031);
    p3 += dot(p3, p3.yzx + 33.33);
    return fract((p3.x + p3.y) * p3.z);
}

vec3 ReconstructWorldPos(vec2 uv, float depth01)
{
    // Vulkan NDC: z in [0, 1]
    vec4 ndc = vec4(uv * 2.0 - 1.0, depth01, 1.0);
    vec4 ws  = inverse(ubo.proj * ubo.view) * ndc;
    return ws.xyz / ws.w;
}

vec3 NormalFromDepth(vec2 uv, float depth01, vec3 P)
{
    ivec2 ts = textureSize(uDepth, 0);
    vec2 du = vec2(1.0 / float(ts.x), 0.0);
    vec2 dv = vec2(0.0, 1.0 / float(ts.y));

    float dR = texture(uDepth, uv + du).r;
    float dU = texture(uDepth, uv + dv).r;

    // If neighbors are missing (sky), fall back to view-facing normal-ish
    if (dR >= 0.999999 || dU >= 0.999999)
    {
        // Approximate normal from view direction (not great, but stable)
        vec3 V = normalize(ubo.camera_position - P);
        return V;
    }

    vec3 PR = ReconstructWorldPos(uv + du, dR);
    vec3 PU = ReconstructWorldPos(uv + dv, dU);

    vec3 N = normalize(cross(PR - P, PU - P));
    // Ensure a consistent orientation (optional)
    return N;
}

void main()
{
    float depth01 = texture(uDepth, vUV).r;

    // Treat far plane / background as no GI
    if (depth01 >= 0.999999)
    {
        oGI = vec4(0.0);
        return;
    }

    vec3 P = ReconstructWorldPos(vUV, depth01);
    vec3 N = NormalFromDepth(vUV, depth01, P);

    // Basic hemisphere basis
    vec3 up = (abs(N.y) < 0.999) ? vec3(0, 1, 0) : vec3(1, 0, 0);
    vec3 T = normalize(cross(up, N));
    vec3 B = cross(N, T);

    // Tunables (start conservative)
    const int   RAYS       = 6;
    const int   STEPS      = 10;
    const float RADIUS     = 1.25;   // world units (depends on your scene scale)
    const float THICKNESS  = 0.04;   // hit tolerance in world units
    const float NORMAL_BIAS = 0.03;  // push off surface to avoid self hits

    // Per-pixel rotation jitter
    float rot = Hash12(gl_FragCoord.xy) * 6.28318530718;

    vec3 gi = vec3(0.0);
    float wsum = 0.0;

    // Offset start point slightly along normal to reduce self-intersections
    vec3 P0 = P + N * NORMAL_BIAS;

    mat4 VP = ubo.proj * ubo.view;

    for (int r = 0; r < RAYS; r++)
    {
        float a = (float(r) / float(RAYS)) * 6.28318530718 + rot;

        // Hemisphere direction (slightly biased towards N)
        vec3 dir = normalize(T * cos(a) + B * sin(a) + N * 0.6);

        for (int s = 1; s <= STEPS; s++)
        {
            float t = (float(s) / float(STEPS)) * RADIUS;
            vec3 Q = P0 + dir * t;

            vec4 clip = VP * vec4(Q, 1.0);
            if (clip.w <= 0.0) continue;

            vec3 ndc = clip.xyz / clip.w;
            vec2 uv  = ndc.xy * 0.5 + 0.5;

            // Outside screen
            if (uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0)
                break;

            float d = texture(uDepth, uv).r;
            if (d >= 0.999999) continue;

            vec3 S = ReconstructWorldPos(uv, d);

            // If our marched point is close to the depth surface, treat as a hit
            float dist = length(S - Q);
            if (dist < THICKNESS)
            {
                vec3 c = texture(uSceneColor, uv).rgb;

                // Weight by how aligned the ray is with the surface normal
                float w = max(dot(N, dir), 0.0);

                gi += c * w;
                wsum += w;
                break;
            }
        }
    }

    if (wsum > 0.0) gi /= wsum;

    // Keep it stable (raw GI can spike)
    gi = clamp(gi, vec3(0.0), vec3(2.0));

    oGI = vec4(gi, 1.0);
}
