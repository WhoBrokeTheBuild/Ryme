#ifndef RYME_GRAPHICS_HPP
#define RYME_GRAPHICS_HPP

#include <Ryme/Config.hpp>
#include <Ryme/Math.hpp>
#include <Ryme/String.hpp>

#include <pybind11/embed.h>
namespace py = pybind11;

#include <SDL.h>

#include <vulkan/vulkan.h>

RYME_DISABLE_WARNINGS()

    #include <vk_mem_alloc.h>

RYME_ENABLE_WARNINGS()

namespace ryme {

///
/// Vulkan Graphics System
///
namespace Graphics {

RYME_API
void ScriptInit(py::module);

RYME_API
void Init(String windowTitle, Vec2i windowSize);

RYME_API
void Term();

} // namespace Graphics

} // namespace ryme

#endif // RYME_GRAPHICS_HPP