#ifndef RYME_SHADER_GLOBALS_HPP
#define RYME_SHADER_GLOBALS_HPP

#include <Ryme/Config.hpp>
#include <Ryme/Math.hpp>

namespace ryme {

struct RYME_API ShaderGlobals
{
    inline static const uint32_t Binding = 0;

    // Viewport resolution (in pixels)
    alignas(8) Vec2 Resolution;
    
    // Mouse location (in pixels)
    alignas(8) Vec2 Mouse;
    
    // Number of frames that have been rendered
    alignas(4) int FrameCount;
    
    // Time passed since initialization (in seconds)
    alignas(4) float TotalTime;

    // Time passed since the previous frame (in seconds)
    alignas(4) float DeltaTime;
    
    // A ratio of DeltaTime / ExpectedDeltaTime
    alignas(4) float FrameSpeedRatio;

}; // struct ShaderGlobals

static_assert(
    sizeof(ShaderGlobals) == 32,
    "sizeof(ShaderGlobals) does not match GLSL layout std140"
);

} // namespace ryme

#endif // RYME_SHADER_GLOBALS_HPP