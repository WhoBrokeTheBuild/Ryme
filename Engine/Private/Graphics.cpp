#include <Ryme/Graphics.hpp>
#include <Ryme/Exception.hpp>
#include <Ryme/Log.hpp>
#include <Ryme/Ryme.hpp>

namespace ryme {

namespace Graphics {

SDL_Window * _sdlWindow = nullptr;

VkInstance _vkInstance = VK_NULL_HANDLE;

void initWindow(const InitInfo& initInfo);
void termWindow();
void initInstance(const InitInfo& initInfo);
void termInstance();

RYME_API
void Init(InitInfo initInfo)
{
    initWindow(initInfo);
    initInstance(initInfo);
}

RYME_API
void Term()
{
    termInstance();
    termWindow();
}

void initWindow(const InitInfo& initInfo)
{
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        throw Exception("SDL_Init failed, {}", SDL_GetError());
    }
    
    SDL_version sdlVersion;
    SDL_GetVersion(&sdlVersion);
    Log(RYME_ANCHOR, "SDL Version: {}.{}.{}",
        sdlVersion.major,
        sdlVersion.minor,
        sdlVersion.patch);
    
    _sdlWindow = SDL_CreateWindow(
        initInfo.WindowTitle.c_str(),
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        initInfo.WindowSize.x,
        initInfo.WindowSize.y,
        SDL_WINDOW_VULKAN
    );

    if (!_sdlWindow) {
        throw Exception("SDL_CreateWindow failed, {}", SDL_GetError());
    }
}

void termWindow()
{
    if (_sdlWindow) {
        SDL_DestroyWindow(_sdlWindow);
        _sdlWindow = nullptr;
    }

    SDL_Quit();
}

void initInstance(const InitInfo& initInfo)
{
    VkResult vkResult;

    const auto& engineVersion = GetVersion();

    VkApplicationInfo applicationInfo = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pNext = nullptr,
        .pApplicationName = initInfo.AppName.c_str(),
        .applicationVersion = VK_MAKE_VERSION(
            initInfo.AppVersion.Major,
            initInfo.AppVersion.Minor,
            initInfo.AppVersion.Patch
        ),
        .pEngineName = "Noon",
        .engineVersion = VK_MAKE_VERSION(
            engineVersion.Major,
            engineVersion.Minor,
            engineVersion.Patch
        ),
        .apiVersion = VK_API_VERSION_1_2,
    };

    // TODO: Layers
    // TODO: Extensions

    VkInstanceCreateInfo instanceCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .pApplicationInfo = &applicationInfo,
        .enabledLayerCount = 0,
        .ppEnabledLayerNames = nullptr,
        .enabledExtensionCount = 0,
        .ppEnabledExtensionNames = nullptr,
    };
    vkResult = vkCreateInstance(&instanceCreateInfo, nullptr, &_vkInstance);
    if (vkResult != VK_SUCCESS) {
        throw Exception("vkCreateInstance() failed");
    }

    Log(RYME_ANCHOR, "Vulkan Version: {}.{}",
        VK_VERSION_MAJOR(VK_HEADER_VERSION_COMPLETE),
        VK_VERSION_MINOR(VK_HEADER_VERSION_COMPLETE));
}

void termInstance()
{
    if (!_vkInstance) {
        vkDestroyInstance(_vkInstance, nullptr);
        _vkInstance = VK_NULL_HANDLE;
    }
}

} // namespace Graphics

} // namespace ryme