#include <Ryme/Math.hpp>
#include <Ryme/Exception.hpp>

#include <pybind11/numpy.h>
#include <pybind11/operators.h>

#include <fmt/format.h>

namespace ryme {

namespace Math {

template <typename T, glm::qualifier Q = glm::packed>
void BindQuaternion(py::module m, const char * name)
{
    using Quat = glm::qua<T, Q>;

    auto c = py::class_<Quat>(m, name, py::buffer_protocol())
        
        .def(py::init())

        .def(py::init<Quat>())

        .def(py::init<Mat3>())

        .def(py::init<Mat4>())

        .def(py::init<T, T, T, T>())

        .def(py::init(
            [](py::list values) {
                if (values.size() != 4) {
                    throw Exception("Invalid size {}, expected {}", values.size(), 4);
                }

                Quat quat;
                for (size_t i = 0; i < 4; ++i) {
                    quat[i] = values[i].cast<T>();
                }
                return quat;
            }))

        .def(py::init(
            [](py::array_t<T> values) {
                if (values.ndim() != 1) {
                    throw Exception("Invalid ndim {}, expected {}", values.ndim(), 1);
                }

                if (values.shape(0) != 4) {
                    throw Exception("Invalid shape {}, expected {}", values.shape(0), 4);
                }

                Quat quat;
                memmove(glm::value_ptr(quat), values.mutable_data(), sizeof(quat));
                return quat;
            }))

        .def_buffer(
            [](Quat& quat) -> py::buffer_info {
                return py::buffer_info(
                    glm::value_ptr(quat),
                    sizeof(T),
                    py::format_descriptor<T>::format(),
                    1,
                    { 4 },
                    { sizeof(T) }
                );
            })

        .def("__getitem__",
            [](Quat& quat, int index) {
                return &quat[index];
            },
            py::return_value_policy::reference_internal)
        
        .def("__setitem__",
            [](Quat& quat, int index, T& value) {
                quat[index] = value;
            })

        .def(py::self == py::self)
        .def(py::self != py::self)

        .def(py::self * py::self)
        .def(py::self + py::self)
        .def(py::self - py::self)
        .def(py::self *= py::self)
        .def(py::self += py::self)
        .def(py::self -= py::self)
        .def(py::self * T())
        .def(py::self / T())
        .def(py::self *= T())
        .def(py::self /= T())

        .def("ToMat3", glm::mat3_cast<T, Q>)
        .def("ToMat4", glm::mat4_cast<T, Q>)
        .def("Dot", glm::dot<T, Q>)
        .def("Cross",
            [](const Quat& quat1, const Quat& quat2) {
                return glm::cross(quat1, quat2);
            })
        .def("Length",
            [](const Quat& quat) {
                return glm::length(quat);
            })
        .def("Inverse",
            [](const Quat& quat) {
                return glm::inverse(quat);
            })
        .def("Normalize",
            [](const Quat& quat) {
                return glm::normalize(quat);
            })
        .def("Pitch", glm::pitch<T, Q>)
        .def("Yaw", glm::yaw<T, Q>)
        .def("Roll", glm::roll<T, Q>)
        .def("EulerAngles", glm::eulerAngles<T, Q>)

        .def("__repr__",
            [](Quat& quat) {
                return fmt::format("ryme.{}", glm::to_string(quat));
            });

    c.def_property("w",
        [](const Quat& quat) { return quat[3]; },
        [](Quat& quat, T value) { quat[3] = value; });
    c.def_property("z",
        [](const Quat& quat) { return quat[2]; },
        [](Quat& quat, T value) { quat[2] = value; });
    c.def_property("y",
        [](const Quat& quat) { return quat[1]; },
        [](Quat& quat, T value) { quat[1] = value; });
    c.def_property("x",
        [](const Quat& quat) { return quat[0]; },
        [](Quat& quat, T value) { quat[0] = value; });
        
    py::implicitly_convertible<py::list, Quat>();
    py::implicitly_convertible<py::array_t<T>, Quat>();
}

void BindQuat(py::module m)
{
    BindQuaternion<float>(m, "Quat");
}

} // namespace Math

} // namespace ryme