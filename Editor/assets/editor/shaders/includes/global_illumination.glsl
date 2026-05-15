#ifndef GLOBAL_ILLUMINATION_GLSL
#define GLOBAL_ILLUMINATION_GLSL

// ================================
// Structures
// ================================

struct GlobalIlluminationSettings
{
    uint enabled;
    uint sample_count;
    float ray_distance;
    float intensity;
    float thickness;
    float falloff;
    float bias;
    float temporal_weight;
};

#endif // GLOBAL_ILLUMINATION_GLSL
