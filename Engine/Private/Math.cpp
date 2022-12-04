#include <Ryme/Math.hpp>

namespace ryme {

namespace Math {

void bindVec2(py::module);
void bindVec3(py::module);
void bindVec4(py::module);
void bindMat(py::module);
void bindQuat(py::module);

RYME_API
void ScriptInit(py::module m)
{
    bindVec2(m);
    bindVec3(m);
    bindVec4(m);
    bindMat(m);
    bindQuat(m);

    m.def("Degrees", glm::degrees<float>)
        .def("Radians", glm::radians<float>)
        .def("Perspective", glm::perspective<float>)
        .def("QuatLookAt", glm::quatLookAt<float, glm::packed>)
        .def("LookAt", glm::lookAt<float, glm::packed>);
}

} // namespace Math

} // namespace ryme
