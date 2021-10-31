#ifndef RYME_TRANSFORM_HPP
#define RYME_TRANSFORM_HPP

#include <Ryme/Config.hpp>
#include <Ryme/Math.hpp>

namespace ryme {

struct RYME_API Transform
{
    Vec3 Position;

    Quat Orientation;

    Vec3 Scale;

    Mat4 ToMatrix() const;

    constexpr Transform& operator+=(const Transform& rhs)
    {
        Position += rhs.Position;
        Orientation *= rhs.Orientation;
        Scale *= rhs.Scale;
        return *this;
    }
};

RYME_API
constexpr Transform operator+(const Transform& lhs, const Transform& rhs)
{
    return Transform{
        lhs.Position + rhs.Position,
        lhs.Orientation * rhs.Orientation,
        lhs.Scale * rhs.Scale
    };
}

} // namespace ryme

#endif // RYME_TRANSFORM_HPP