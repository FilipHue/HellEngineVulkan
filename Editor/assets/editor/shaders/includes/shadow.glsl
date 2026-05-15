#ifndef SHADOW_GLSL
#define SHADOW_GLSL

// ================================
// Structures
// ================================

struct ShadowSettings
{
    uint enabled;
    uint shadow_map_size;
    float cascade_split_lambda;
    float min_bias;
    float max_bias;
    float normal_offset;
    uint pcf_samples;
    float softness;
    vec4 cascade_distances;
};

#endif // SHADOW_GLSL
