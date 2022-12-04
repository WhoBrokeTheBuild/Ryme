#include "Math.Vec.hpp"

namespace ryme {

namespace Math {

void bindVec4(py::module m)
{
    bindVector<4, float>(m, "Vec4");
    bindVector<4, int32_t>(m, "Vec4i");
    bindVector<4, uint32_t>(m, "Vec4u");
}

} // namespace Math

} // namespace ryme
