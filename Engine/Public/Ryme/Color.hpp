#ifndef RYME_COLOR_SPACE_HPP
#define RYME_COLOR_SPACE_HPP

#include <Ryme/Config.hpp>
#include <Ryme/Array.hpp>
#include <Ryme/Math.hpp>
#include <Ryme/String.hpp>

#include <Ryme/ThirdParty/fmt.hpp>
#include <Ryme/ThirdParty/python.hpp>

namespace ryme {

namespace Color {

// Convert a color from Linear to sRGB Color Space
RYME_API
Vec4 ToSRGB(const Vec4& color);

// Convert a color from sRGB to Linear Color Space
RYME_API
Vec4 ToLinear(const Vec4& color);

constexpr inline Vec4 FromInts(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha = 255)
{
    return {
        red   / 255.0f,
        green / 255.0f,
        blue  / 255.0f,
        alpha / 255.0f
    };
}

constexpr inline Vec4u ToInts(const Vec4& color)
{
    return Vec4u(
        color.r * 255,
        color.g * 255,
        color.b * 255,
        color.a * 255
    );
}

constexpr inline Vec4 FromHex(uint32_t color)
{
    return Color::FromInts(
        ((color & 0xFF000000) >> 24),
        ((color & 0x00FF0000) >> 16),
        ((color & 0x0000FF00) >> 8),
        ((color & 0x000000FF))
    );
}

constexpr inline uint32_t ToHex(const Vec4& color)
{
    return (
          ((uint32_t)(color.r * 255) << 24)
        | ((uint32_t)(color.g * 255) << 16)
        | ((uint32_t)(color.b * 255) << 8)
        |  (uint32_t)(color.a * 255)
    );
}

constexpr inline Array<float, 4> ToArray(const Vec4& color)
{
    return Array<float, 4>{ color.r, color.g, color.b, color.a };
}

RYME_API
void ScriptInit(py::module);

constexpr Vec4 Transparent    = FromInts(0,   0,   0,   0);
constexpr Vec4 Black          = FromInts(0,   0,   0  );
constexpr Vec4 White          = FromInts(255, 255, 255);
constexpr Vec4 Red            = FromInts(255, 0,   0  );
constexpr Vec4 Green          = FromInts(0,   255, 0  );
constexpr Vec4 Blue           = FromInts(0,   0,   255);
constexpr Vec4 Yellow         = FromInts(255, 255, 0  );
constexpr Vec4 Magenta        = FromInts(255, 0,   255);
constexpr Vec4 Cyan           = FromInts(0,   255, 255);
constexpr Vec4 CornflowerBlue = FromInts(100, 149, 237); // https://en.wikipedia.org/wiki/Cornflower_blue

} // namespace Color

} // namespace ryme

#endif // RYME_COLOR_SPACE_HPP