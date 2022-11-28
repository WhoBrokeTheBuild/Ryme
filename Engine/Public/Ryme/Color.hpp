#ifndef RYME_COLOR_SPACE_HPP
#define RYME_COLOR_SPACE_HPP

#include <Ryme/Config.hpp>
#include <Ryme/Containers.hpp>
#include <Ryme/Math.hpp>
#include <Ryme/String.hpp>

#include <Ryme/ThirdParty/fmt.hpp>

namespace ryme {

class Color
{
public:

    float R = 0.0f;

    float G = 0.0f;

    float B = 0.0f;

    float A = 1.0f;

    constexpr Color() = default;

    constexpr explicit Color(float red, float green, float blue, float alpha = 1.0f)
        : R(red)
        , G(green)
        , B(blue)
        , A(alpha)
    { }

    constexpr Color(int red, int green, int blue, int alpha = 255)
        : R(red / 255.0f)
        , G(green / 255.0f)
        , B(blue / 255.0f)
        , A(alpha / 255.0f)
    { }

    inline Array<float, 4> ToArray() const
    {
        return Array<float, 4>{ R, G, B, A };
    }
    
    inline Vec4 ToVec4() const
    {
        return Vec4(R, G, B, A);
    }
    
    // https://en.wikipedia.org/wiki/SRGB

    inline Color ToSRGB() const
    {
        return Color(
            (R < 0.0031308f ? 12.92f * R : 1.055f * pow(R, 1.0f / 2.4f) - 0.055f),
            (G < 0.0031308f ? 12.92f * G : 1.055f * pow(G, 1.0f / 2.4f) - 0.055f),
            (B < 0.0031308f ? 12.92f * B : 1.055f * pow(B, 1.0f / 2.4f) - 0.055f),
            A
        );
    }

    inline Color ToLinear() const
    {
        return Color(
            (R < 0.04045f ? (1.0f / 12.92f) * R : pow((R + 0.055f) * (1.0f / 1.055f), 2.4f)),
            (G < 0.04045f ? (1.0f / 12.92f) * G : pow((G + 0.055f) * (1.0f / 1.055f), 2.4f)),
            (B < 0.04045f ? (1.0f / 12.92f) * B : pow((B + 0.055f) * (1.0f / 1.055f), 2.4f)),
            A
        );
    }

    inline String ToString() const
    {
        return fmt::format("{:0.3f}, {:0.3f}, {:0.3f}, {:0.3f} ({}, {}, {}, {})",
            R, G, B, A,
            R * 255, G * 255, B * 255, A * 255
        );
    }

    static const Color Transparent;
    static const Color Black;
    static const Color White;
    static const Color Red;
    static const Color Green;
    static const Color Blue;
    static const Color Yellow;
    static const Color Magenta;
    static const Color Cyan;
    
    // https://en.wikipedia.org/wiki/Cornflower_blue
    static const Color CornflowerBlue;

}; // class Color

inline constexpr Color Color::Transparent      (0,   0,   0,   0);
inline constexpr Color Color::Black            (0,   0,   0  );
inline constexpr Color Color::White            (255, 255, 255);
inline constexpr Color Color::Red              (255, 0,   0  );
inline constexpr Color Color::Green            (0,   255, 0  );
inline constexpr Color Color::Blue             (0,   0,   255);
inline constexpr Color Color::Yellow           (255, 255, 0  );
inline constexpr Color Color::Magenta          (255, 0,   255);
inline constexpr Color Color::Cyan             (0,   255, 255);
inline constexpr Color Color::CornflowerBlue   (100, 149, 237);

} // namespace ryme

template <>
struct fmt::formatter<ryme::Color> : public fmt::formatter<std::string_view>
{
    template <typename FormatContext>
    auto format(const ryme::Color& color, FormatContext& ctx) {
        return formatter<std::string_view>::format(std::string_view(color.ToString()), ctx);
    }
};

#endif // RYME_COLOR_SPACE_HPP