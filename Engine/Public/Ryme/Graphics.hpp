#ifndef RYME_GRAPHICS_HPP
#define RYME_GRAPHICS_HPP

#include <Ryme/Config.hpp>
#include <Ryme/String.hpp>
#include <Ryme/Math.hpp>
#include <Ryme/Version.hpp>

#include <SDL.h>

#include <vulkan/vulkan.h>

RYME_DISABLE_WARNINGS()

    #include <vk_mem_alloc.h>

RYME_ENABLE_WARNINGS()

namespace ryme {

namespace Graphics {

RYME_API
void Init(String windowTitle, Vec2i windowSize);

RYME_API
void Term();

} // namespace Graphics

} // namespace ryme

#endif // RYME_GRAPHICS_HPP