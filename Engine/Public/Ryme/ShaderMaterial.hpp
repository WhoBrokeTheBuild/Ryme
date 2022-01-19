#ifndef RYME_SHADER_MATERIAL_HPP
#define RYME_SHADER_MATERIAL_HPP

#include <Ryme/Config.hpp>
#include <Ryme/Math.hpp>

namespace ryme {

struct RYME_API ShaderMaterial
{
    static inline const uint32_t Binding = 2;

    Vec4 BaseColorFactor;

    Vec3 EmissiveFactor;

    float MetallicFactor;

    float RoughnessFactor;

    float OcclusionStrength;

    float NormalScale;

}; // struct ShaderMaterial

static_assert(
    sizeof(ShaderMaterial) == 44,
    "sizeof(ShaderMaterial) does not match GLSL layout std140"
);

// TODO: Textures

} // namespace ryme

#endif // RYME_SHADER_MATERIAL_HPP