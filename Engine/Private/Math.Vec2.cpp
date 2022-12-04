#include "Math.Vec.hpp"

namespace ryme {

namespace Math {

void bindVec2(py::module m)
{
    bindVector<2, float>(m, "Vec2");
    bindVector<2, int32_t>(m, "Vec2i");
    bindVector<2, uint32_t>(m, "Vec2u");
}

} // namespace Math

} // namespace ryme
