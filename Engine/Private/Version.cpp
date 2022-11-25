#include <Ryme/Version.hpp>
#include <Ryme/Script.hpp>

#include <pybind11/operators.h>

namespace ryme {

void Version::FromString(const String& string)
{
    std::smatch match;
    std::regex regex("([0-9])+\\.([0-9]+)\\.([0-9]+)");
    std::regex_search(string, match, regex);

    if (match.size() == 4) {
        Major = std::strtol(match[1].str().c_str(), nullptr, 10);
        Minor = std::strtol(match[2].str().c_str(), nullptr, 10);
        Patch = std::strtol(match[3].str().c_str(), nullptr, 10);
    }
}
    
int Version::Compare(const Version& a, const Version& b)
{
    auto cmp = [](int a, int b)
    {
        if (a == b) {
            return 0;
        }
        if (a > b) {
            return 1;
        }
        return -1;
    };

    int result = cmp(a.Major, b.Major);
    if (result == 0) {
        result = cmp(a.Minor, b.Minor);
        if (result == 0) {
            result = cmp(a.Patch, b.Patch);
        }
    }
    return result;
}

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
        .def(py::self > py::self)
        .def(py::self >= py::self)
        .def(py::self < py::self)
        .def(py::self <= py::self)

        .def("__str__", &Version::ToString)

        .def("__repr__", 
            [](const Version& v) {
                return fmt::format(
                    "ryme.Version({}, {}, {})",
                    v.Major,
                    v.Minor,
                    v.Patch
                );
            });
}

} // namespace ryme