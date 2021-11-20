#include <Ryme/Math.hpp>
#include <Ryme/Log.hpp>
#include <Ryme/Exception.hpp>

#include <pybind11/numpy.h>
#include <pybind11/operators.h>

namespace ryme {

namespace Math {

void BindVec2(py::module);
void BindVec3(py::module);
void BindVec4(py::module);
void BindMat(py::module);
void BindQuat(py::module);

RYME_API
void ScriptInit(py::module m)
{
    BindVec2(m);
    BindVec3(m);
    BindVec4(m);
    BindMat(m);
    BindQuat(m);

    m.def("Degrees", glm::degrees<float>)
        .def("Radians", glm::radians<float>)
        .def("Perspective", glm::perspective<float>)
        .def("QuatLookAt", glm::quatLookAt<float, glm::packed>)
        .def("LookAt", glm::lookAt<float, glm::packed>);
}

} // namespace Math

} // namespace ryme
