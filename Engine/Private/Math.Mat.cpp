#include <Ryme/Math.hpp>
#include <Ryme/Exception.hpp>

#include <pybind11/numpy.h>
#include <pybind11/operators.h>

namespace ryme {

namespace Math {

template <glm::length_t L, typename T, glm::qualifier Q = glm::packed>
void BindMatrix(py::module m, const char * name)
{
    using Matrix = glm::mat<L, L, T, Q>;
    using Vector = typename Matrix::col_type;

    auto c = py::class_<Matrix>(m, name, py::buffer_protocol())
    
        .def(py::init())

        .def(py::init<Matrix>())

        .def(py::init<T>())

        .def(py::init(
            [](py::list rows) {
                if (rows.size() != L) {
                    throw Exception("Invalid row size {}, expected {}", rows.size(), L);
                }

                Matrix mat;
                for (size_t i = 0; i < rows.size(); ++i) {
                    py::list cols = rows[i].cast<py::list>();

                    if (cols.size() != L) {
                        throw Exception("Invalid col size {}, expected {}", rows.size(), L);
                    }

                    for (size_t j = 0; j < cols.size(); ++j) {
                        mat[i][j] = cols[j].cast<T>();
                    }
                }
                return mat;
            }))

        .def(py::init(
            [](py::array_t<T> values) {
                if (values.ndim() != 2) {
                    throw Exception("Invalid ndim {}, expected {}", values.ndim(), 2);
                }

                if (values.shape(0) != L || values.shape(1) != L) {
                    throw Exception("Invalid shape {}x{}, expected {}x{}",
                        values.shape(0), values.shape(1), L, L);
                }

                if (values.strides(0) / sizeof(T) != L || values.strides(1) != sizeof(T)) {
                    throw Exception("Invalid strides, data must be contiguous");
                }

                Matrix mat;
                memmove(glm::value_ptr(mat), values.mutable_data(), sizeof(mat));
                return mat;
            }))

        .def_buffer(
            [](Matrix& mat) -> py::buffer_info {
                return py::buffer_info(
                    glm::value_ptr(mat),
                    sizeof(T),
                    py::format_descriptor<T>::format(),
                    L,
                    { L, L },
                    { sizeof(T) * L, sizeof(T) }
                );
            })

        .def("__getitem__",
            [](Matrix& mat, int index) {
                return &mat[index];
            },
            py::return_value_policy::reference_internal)
        
        .def("__setitem__",
            [](Matrix& mat, int index, Vector& col) {
                mat[index] = col;
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

        .def("Determinant", glm::determinant<L, L, T, Q>)
        .def("Inverse", glm::inverse<L, L, T, Q>)
        .def("Transpose", glm::transpose<L, L, T, Q>)

        .def("__repr__", 
            [](const Matrix& mat) {
                return fmt::format("ryme.{}", glm::to_string(mat));
            });
    
    if constexpr (L >= 3) {
        c.def("ToQuat",
            [](const Matrix& mat) {
                return glm::quat_cast(mat);
            });
    }

    if constexpr (L == 4) {
        c.def("Rotate",
            [](const Mat4& mat, const float& angle, const Vec3& vec) {
                return glm::rotate(mat, angle, vec);
            });
        c.def("Scale",
            [](const Mat4& mat, const Vec3& vec) {
                return glm::scale(mat, vec);
            });
        c.def("Translate",
            [](const Mat4& mat, const Vec3& vec) {
                return glm::translate(mat, vec);
            });
    }
            
    py::implicitly_convertible<py::list, Matrix>();
    py::implicitly_convertible<py::array_t<T>, Matrix>();
}

void BindMat(py::module m)
{
    BindMatrix<2, float>(m, "Mat2");
    BindMatrix<3, float>(m, "Mat3");
    BindMatrix<4, float>(m, "Mat4");

}

} // namespace Math

} // namespace ryme
