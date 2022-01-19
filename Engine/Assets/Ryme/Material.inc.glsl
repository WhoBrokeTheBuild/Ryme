#ifndef RYME_TRANSFORM_INC_GLSL
#define RYME_TRANSFORM_INC_GLSL

layout(binding = 2, std140) uniform RymeMaterial
{
    vec4 u_BaseColorFactor;
    vec3 u_EmissiveFactor;
    float u_MetallicFactor;
    float u_RoughnessFactor;
    float u_OcclusionStrength;
    float u_NormalScale;
};

// TODO: Investigate
layout(set = 1, binding = 0) uniform sampler2D u_BaseColorMap;
layout(set = 1, binding = 1) uniform sampler2D u_NormalMap;
layout(set = 1, binding = 2) uniform sampler2D u_MetallicRoughnessMap;
layout(set = 1, binding = 3) uniform sampler2D u_EmissiveMap;
layout(set = 1, binding = 4) uniform sampler2D u_OcclusionMap;

#endif // RYME_TRANSFORM_INC_GLSL