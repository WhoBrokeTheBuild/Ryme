#include "Math.Vec.hpp"

namespace ryme {

namespace Math {

void BindVec2(py::module m)
{
    BindVector<2, float>(m, "Vec2");
    BindVector<2, int32_t>(m, "Vec2i");
    BindVector<2, uint32_t>(m, "Vec2u");
}

} // namespace Math

} // namespace ryme
