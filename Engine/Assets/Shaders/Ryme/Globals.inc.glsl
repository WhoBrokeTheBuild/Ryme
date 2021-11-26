#ifndef RYME_GLOBALS_INC_GLSL
#define RYME_GLOBALS_INC_GLSL

layout(binding = 0, std140) uniform RymeGlobals
{
    vec2 u_Resolution;
    vec2 u_Mouse;
    int u_FrameCount;
    float u_TotalTime;
    float u_FrameSpeedRatio;
    
};

#endif // RYME_GLOBALS_INC_GLSL