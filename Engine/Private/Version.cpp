#include <Ryme/Version.hpp>
#include <Ryme/Script.hpp>

#include <pybind11/operators.h>

namespace ryme {

void Version::ScriptInit(py::module m)
{
    py::class_<Version>(m, "Version")

        .def(py::init<int, int, int>(),
            py::arg("major") = 0,
            py::arg("minor") = 0,
            py::arg("patch") = 0)

        .def(py::init<const String&>(),
            py::arg("str"))

        .def_readwrite("Major", &Version::Major)
        .def_readwrite("Minor", &Version::Minor)
        .def_readwrite("Patch", &Version::Patch)

        .def(py::self == py::self)
        .def(py::self != py::self)
        .def(py::self < py::self)
        .def(py::self <= py::self)
        .def(py::self > py::self)
        .def(py::self >= py::self)

        .def("__str__", &Version::ToString)

        .def("__repr__", 
            [](const Version& v) {
                return fmt::format(
                    "ryme::Version({}, {}, {})",
                    v.Major,
                    v.Minor,
                    v.Patch
                );
            });
}

} // namespace ryme