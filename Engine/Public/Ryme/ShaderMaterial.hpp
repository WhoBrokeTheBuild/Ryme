#ifndef RYME_SHADER_MATERIAL_HPP
#define RYME_SHADER_MATERIAL_HPP

#include <Ryme/Config.hpp>
#include <Ryme/Math.hpp>

namespace ryme {

struct RYME_API ShaderMaterial
{
public:

    static inline const uint32_t Binding = 2;

    alignas(4) Vec4 BaseColorFactor;

    alignas(4) Vec3 EmissiveFactor;

    alignas(4) float MetallicFactor;

    alignas(4) float RoughnessFactor;

    alignas(4) float OcclusionStrength;

    alignas(4) float NormalScale;

}; // struct ShaderMaterial

static_assert(
    sizeof(ShaderMaterial) == 44,
    "sizeof(ShaderMaterial) does not match GLSL layout std140"
);

// TODO: Textures

} // namespace ryme

#endif // RYME_SHADER_MATERIAL_HPP