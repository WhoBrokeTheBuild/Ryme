#include <Ryme/Math.hpp>
#include <Ryme/Exception.hpp>

#include <Ryme/ThirdParty/fmt.hpp>

#include <pybind11/numpy.h>
#include <pybind11/operators.h>

namespace ryme {

namespace Math {

template <glm::length_t L, typename T, glm::qualifier Q = glm::packed>
void bindVector(py::module m, const char * name)
{
    using Vector = glm::vec<L, T, Q>;

    auto c = py::class_<Vector>(m, name, py::buffer_protocol())
        
        .def(py::init())

        .def(py::init<Vector>())

        .def(py::init<T>())

        .def(py::init(
            [](py::list values) {
                if (values.size() != L) {
                    throw Exception("Invalid size {}, expected {}", values.size(), L);
                }

                Vector vec;
                for (size_t i = 0; i < L; ++i) {
                    vec[i] = values[i].cast<T>();
                }
                return vec;
            }))

        .def(py::init(
            [](py::array_t<T> values) {
                if (values.ndim() != 1) {
                    throw Exception("Invalid ndim {}, expected {}", values.ndim(), 1);
                }

                if (values.shape(0) != L) {
                    throw Exception("Invalid shape {}, expected {}", values.shape(0), L);
                }

                Vector vec;
                memmove(glm::value_ptr(vec), values.mutable_data(), sizeof(vec));
                return vec;
            }))

        .def_buffer(
            [](Vector& vec) -> py::buffer_info {
                return py::buffer_info(
                    glm::value_ptr(vec),
                    sizeof(T),
                    py::format_descriptor<T>::format(),
                    1,
                    { L },
                    { sizeof(T) }
                );
            })

        .def("__getitem__",
            [](Vector& vec, int index) {
                return &vec[index];
            },
            py::return_value_policy::reference_internal)
        
        .def("__setitem__",
            [](Vector& vec, int index, T& value) {
                vec[index] = value;
            })

        .def(py::self == py::self)
        .def(py::self != py::self)

        .def("__lt__", glm::lessThan<L, T, Q>)
        .def("__le__", glm::lessThanEqual<L, T, Q>)
        .def("__gt__", glm::greaterThan<L, T, Q>)
        .def("__ge__", glm::greaterThanEqual<L, T, Q>)

        .def(py::self * py::self)
        .def(py::self / py::self)
        .def(py::self + py::self)
        .def(py::self - py::self)
        .def(py::self *= py::self)
        .def(py::self /= py::self)
        .def(py::self += py::self)
        .def(py::self -= py::self)
        .def(py::self + T())
        .def(py::self - T())
        .def(py::self * T())
        .def(py::self / T())
        .def(py::self += T())
        .def(py::self -= T())
        .def(py::self *= T())
        .def(py::self /= T())

        .def("Clamp", 
            [](const Vector& vec1, const Vector& vec2, const Vector& vec3) {
                return glm::clamp(vec1, vec2, vec3);
            })
            
        .def("Clamp", 
            [](const Vector& vec, const T& minValue, const T& maxValue) {
                return glm::clamp(vec, minValue, maxValue);
            })

        .def("Abs", glm::abs<L, T, Q>)
        .def("Max",
            [](const Vector& vec1, const Vector& vec2) {
                return glm::max(vec1, vec2);
            })
        .def("Min",
            [](const Vector& vec1, const Vector& vec2) {
                return glm::min(vec1, vec2);
            })

        .def("__repr__",
            [](Vector& vec) {
                return fmt::format("ryme.{}", glm::to_string(vec));
            });

    if constexpr (L == 2) {
        c.def(py::init<T, T>());
    }

    if constexpr (L == 3) {
        c.def(py::init<T, T, T>());
    }

    if constexpr (L == 4) {
        c.def(py::init<T, T, T, T>());
    }

    if constexpr (std::is_floating_point_v<T>) {
        c.def("Asin", glm::asin<L, T, Q>);
        c.def("Acos", glm::acos<L, T, Q>);
        c.def("Atan", 
            [](const Vector& vec) {
                return glm::atan(vec);
            });
        c.def("Atan2",
            [](const Vector& vec1, const Vector& vec2) {
                return glm::atan(vec1, vec2);
            });
        c.def("Sin", glm::sin<L, T, Q>);
        c.def("Cos", glm::cos<L, T, Q>);
        c.def("Tan", glm::tan<L, T, Q>);
        c.def("Dot", glm::dot<L, T, Q>);
        c.def("Ceil", glm::ceil<L, T, Q>);
        c.def("Floor", glm::floor<L, T, Q>);
        c.def("Round", glm::round<L, T, Q>);
        c.def("Distance", glm::distance<L, T, Q>);
        c.def("Distance2", glm::distance2<L, T, Q>);
        c.def("Length", glm::length<L, T, Q>);
        c.def("Length2", glm::length2<L, T, Q>);
        c.def("Normalize", glm::normalize<L, T, Q>);

        if constexpr (L == 3) {
            c.def("Cross", 
                [](const Vector& vec1, const Vector& vec2) {
                    return glm::cross(vec1, vec2);
                });
        }
    }
            
    switch (L) {
    case 4:
        c.def_property("w",
            [](const Vector& vec) { return vec[3]; },
            [](Vector& vec, T value) { vec[3] = value; });
        c.def_property("a",
            [](const Vector& vec) { return vec[3]; },
            [](Vector& vec, T value) { vec[3] = value; });
        c.def_property("q",
            [](const Vector& vec) { return vec[3]; },
            [](Vector& vec, T value) { vec[3] = value; });
        [[fallthrough]];
    case 3:
        c.def_property("z",
            [](const Vector& vec) { return vec[2]; },
            [](Vector& vec, T value) { vec[2] = value; });
        c.def_property("b",
            [](const Vector& vec) { return vec[2]; },
            [](Vector& vec, T value) { vec[2] = value; });
        c.def_property("p",
            [](const Vector& vec) { return vec[2]; },
            [](Vector& vec, T value) { vec[2] = value; });
        [[fallthrough]];
    case 2:
        c.def_property("y",
            [](const Vector& vec) { return vec[1]; },
            [](Vector& vec, T value) { vec[1] = value; });
        c.def_property("g",
            [](const Vector& vec) { return vec[1]; },
            [](Vector& vec, T value) { vec[1] = value; });
        c.def_property("t",
            [](const Vector& vec) { return vec[1]; },
            [](Vector& vec, T value) { vec[1] = value; });

        c.def_property("x",
            [](const Vector& vec) { return vec[0]; },
            [](Vector& vec, T value) { vec[0] = value; });
        c.def_property("r",
            [](const Vector& vec) { return vec[0]; },
            [](Vector& vec, T value) { vec[0] = value; });
        c.def_property("s",
            [](const Vector& vec) { return vec[0]; },
            [](Vector& vec, T value) { vec[0] = value; });
    default: ;
    }
    
    py::implicitly_convertible<py::list, Vector>();
    py::implicitly_convertible<py::array_t<T>, Vector>();
}

} // namespace Math

} // namespace ryme