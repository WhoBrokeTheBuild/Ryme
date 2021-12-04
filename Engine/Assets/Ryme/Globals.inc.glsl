#ifndef RYME_GLOBALS_INC_GLSL
#define RYME_GLOBALS_INC_GLSL

layout(binding = 0, std140) uniform RymeGlobals
{
    // Viewport resolution (in pixels)
    vec2 u_Resolution;

    // Mouse location (in pixels)
    vec2 u_Mouse;

    // Number of frames that have been rendered
    int u_FrameCount;

    // Time passed since initialization (in seconds)
    float u_TotalTime;

    // Time passed since the previous frame (in seconds)
    float u_DeltaTime;

    // A ratio of DeltaTime / ExpectedDeltaTime
    float u_FrameSpeedRatio;
    
};

#endif // RYME_GLOBALS_INC_GLSL