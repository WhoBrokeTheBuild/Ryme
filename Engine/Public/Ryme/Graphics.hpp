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

struct RYME_API InitInfo
{
    String WindowTitle = RYME_PROJECT_NAME " (" RYME_VERSION_STRING ")";

    Vec2i WindowSize = { 640, 480 };

    String AppName;

    Version AppVersion;

}; // struct InitInfo

RYME_API
void Init(InitInfo initInfo = {});

RYME_API
void Term();

} // namespace Graphics

} // namespace ryme

#endif // RYME_GRAPHICS_HPP