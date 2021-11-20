#ifndef RYME_MATH_HPP
#define RYME_MATH_HPP

#include <Ryme/Config.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_RIGHT_HANDED
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include <glm/gtc/type_ptr.hpp>

#include <pybind11/embed.h>
namespace py = pybind11;

#include <limits>

namespace ryme {

using Quat = glm::qua<float, glm::packed>;

using Mat2 = glm::mat<2, 2, float, glm::packed>;
using Mat3 = glm::mat<3, 3, float, glm::packed>;
using Mat4 = glm::mat<4, 4, float, glm::packed>;

using Vec2 = glm::vec<2, float, glm::packed>;
using Vec3 = glm::vec<3, float, glm::packed>;
using Vec4 = glm::vec<4, float, glm::packed>;

using Vec2i = glm::vec<2, int32_t, glm::packed>;
using Vec3i = glm::vec<3, int32_t, glm::packed>;
using Vec4i = glm::vec<4, int32_t, glm::packed>;

using Vec2u = glm::vec<2, uint32_t, glm::packed>;
using Vec3u = glm::vec<3, uint32_t, glm::packed>;
using Vec4u = glm::vec<4, uint32_t, glm::packed>;

template <typename T>
constexpr float NormalizeInteger(T value)
{
    return (
        value < 0
        ? -static_cast<float>(value) / static_cast<float>(std::numeric_limits<T>::min())
        :  static_cast<float>(value) / static_cast<float>(std::numeric_limits<T>::min())
    );
}

namespace Math {

RYME_API
void ScriptInit(py::module);

} // namespace Math

} // namespace ryme

PYBIND11_MAKE_OPAQUE(ryme::Vec2)
PYBIND11_MAKE_OPAQUE(ryme::Vec3)
PYBIND11_MAKE_OPAQUE(ryme::Vec4)

PYBIND11_MAKE_OPAQUE(ryme::Mat2)
PYBIND11_MAKE_OPAQUE(ryme::Mat3)
PYBIND11_MAKE_OPAQUE(ryme::Mat4)

#endif // RYME_MATH_HPP