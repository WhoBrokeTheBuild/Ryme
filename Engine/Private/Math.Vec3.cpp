#include "Math.Vec.hpp"

namespace ryme {

namespace Math {

void BindVec3(py::module m)
{
    BindVector<3, float>(m, "Vec3");
    BindVector<3, int32_t>(m, "Vec3i");
    BindVector<3, uint32_t>(m, "Vec3u");
}

} // namespace Math

} // namespace ryme
