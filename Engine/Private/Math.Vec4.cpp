#include "Math.Vec.hpp"

namespace ryme {

namespace Math {

void BindVec4(py::module m)
{
    BindVector<4, float>(m, "Vec4");
    BindVector<4, int32_t>(m, "Vec4i");
    BindVector<4, uint32_t>(m, "Vec4u");
}

} // namespace Math

} // namespace ryme
