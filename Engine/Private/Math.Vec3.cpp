#include "Math.Vec.hpp"

namespace ryme {

namespace Math {

void bindVec3(py::module m)
{
    bindVector<3, float>(m, "Vec3");
    bindVector<3, int32_t>(m, "Vec3i");
    bindVector<3, uint32_t>(m, "Vec3u");
}

} // namespace Math

} // namespace ryme
