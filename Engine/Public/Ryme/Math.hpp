#ifndef RYME_MATH_HPP
#define RYME_MATH_HPP

#include <Ryme/Config.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_RIGHT_HANDED
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include <limits>

namespace ryme {

using Quat = glm::quat;

using Mat2 = glm::mat2;
using Mat3 = glm::mat3;
using Mat4 = glm::mat4;

using Vec2 = glm::vec2;
using Vec3 = glm::vec3;
using Vec4 = glm::vec4;

using Vec2b = glm::bvec2;
using Vec3b = glm::bvec3;
using Vec4b = glm::bvec4;

using Vec2i = glm::ivec2;
using Vec3i = glm::ivec3;
using Vec4i = glm::ivec4;

using Vec2u = glm::uvec2;
using Vec3u = glm::uvec3;
using Vec4u = glm::uvec4;

template <typename T>
constexpr float NormalizeInteger(T value)
{
    return (
        value < 0
        ? -static_cast<float>(value) / static_cast<float>(std::numeric_limits<T>::min())
        :  static_cast<float>(value) / static_cast<float>(std::numeric_limits<T>::min())
    );
}

} // namespace ryme

#endif // RYME_MATH_HPP