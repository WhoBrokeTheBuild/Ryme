#include <Ryme/Ryme.hpp>
#include <Ryme/Log.hpp>

#include <pybind11/embed.h>
namespace py = pybind11;

template<>
struct fmt::formatter<py::str> : public fmt::formatter<std::string_view>
{
    template <typename FormatContext>
    auto format(const py::str& str, FormatContext& ctx) {
        return formatter<std::string_view>::format(str.cast<std::string_view>(), ctx);
    }
};

#include <vulkan/vulkan.h>

RYME_DISABLE_WARNINGS()
#include <vk_mem_alloc.h>
RYME_ENABLE_WARNINGS()

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include <SDL.h>

int main(int argc, char ** argv)
{
    try {
        ryme::Log(RYME_ANCHOR, "Ryme Version: {}", ryme::GetVersion());

        ryme::Log(RYME_ANCHOR, "{{fmt}} Version: {}.{}.{}",
            FMT_VERSION / 10000,
            (FMT_VERSION % 10000) / 100,
            (FMT_VERSION % 10000) % 100);

        py::scoped_interpreter guard{};
        py::object pyVersion = py::module_::import("sys").attr("version");
        ryme::Log(RYME_ANCHOR, "Python Version: {}",
            pyVersion.str());

        VkResult vkResult;
        VkInstance vkInstance;
        VkInstanceCreateInfo instanceCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .pApplicationInfo = nullptr,
            .enabledLayerCount = 0,
            .ppEnabledLayerNames = nullptr,
            .enabledExtensionCount = 0,
            .ppEnabledExtensionNames = nullptr,
        };
        vkResult = vkCreateInstance(&instanceCreateInfo, nullptr, &vkInstance);
        if (vkResult != VK_SUCCESS) {
            throw ryme::Exception("vkCreateInstance() failed");
        }

        ryme::Log(RYME_ANCHOR, "Vulkan Version: {}.{}",
            VK_VERSION_MAJOR(VK_HEADER_VERSION_COMPLETE),
            VK_VERSION_MINOR(VK_HEADER_VERSION_COMPLETE));

        ryme::Log(RYME_ANCHOR, "GLM Version: {}.{}.{}.{}",
            GLM_VERSION_MAJOR,
            GLM_VERSION_MINOR,
            GLM_VERSION_PATCH,
            GLM_VERSION_REVISION);

        if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
            throw ryme::Exception("SDL_Init() failed");
        }

        SDL_version sdlVersion;
        SDL_GetVersion(&sdlVersion);
        ryme::Log(RYME_ANCHOR, "SDL2 Version: {}.{}.{}",
            sdlVersion.major,
            sdlVersion.minor,
            sdlVersion.patch);
    }
    catch (const std::exception& e) {
        ryme::Log(RYME_ANCHOR, "Exception {}", e.what());
    }

    return 0;
}