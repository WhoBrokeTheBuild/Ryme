#include <Ryme/Color.hpp>

#include <pybind11/operators.h>

namespace ryme {

namespace Color {

// https://en.wikipedia.org/wiki/SRGB

RYME_API
Vec4 ToSRGB(const Vec4& color)
{
    return Vec4(
        (color.r < 0.0031308f ? 12.92f * color.r : 1.055f * pow(color.r, 1.0f / 2.4f) - 0.055f),
        (color.g < 0.0031308f ? 12.92f * color.g : 1.055f * pow(color.g, 1.0f / 2.4f) - 0.055f),
        (color.b < 0.0031308f ? 12.92f * color.b : 1.055f * pow(color.b, 1.0f / 2.4f) - 0.055f),
        color.a
    );
}

RYME_API
Vec4 ToLinear(const Vec4& color)
{
    return Vec4(
        (color.r < 0.04045f ? (1.0f / 12.92f) * color.r : pow((color.r + 0.055f) * (1.0f / 1.055f), 2.4f)),
        (color.g < 0.04045f ? (1.0f / 12.92f) * color.g : pow((color.g + 0.055f) * (1.0f / 1.055f), 2.4f)),
        (color.b < 0.04045f ? (1.0f / 12.92f) * color.b : pow((color.b + 0.055f) * (1.0f / 1.055f), 2.4f)),
        color.a
    );
}

RYME_API
void ScriptInit(py::module m)
{
    m.def_submodule("Color")
        .def("ToSRGB", &ToSRGB)
        .def("ToLinear", &ToLinear)
        .def("FromInts", &FromInts)
        .def("ToInts", &ToInts)
        .def("FromHex", &FromHex)
        .def("ToHex", &ToHex)
        // TODO: Improve?
        .def("Transparent", []() { return Transparent; })
        .def("Black", []() { return Black; })
        .def("White", []() { return White; })
        .def("Red", []() { return Red; })
        .def("Green", []() { return Green; })
        .def("Blue", []() { return Blue; })
        .def("Yellow", []() { return Yellow; })
        .def("Magenta", []() { return Magenta; })
        .def("Cyan", []() { return Cyan; })
        .def("CornflowerBlue", []() { return CornflowerBlue; });
}

} // namespace Color

} // namespace ryme