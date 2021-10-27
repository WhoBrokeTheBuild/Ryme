#include <Ryme/Graphics.hpp>
#include <Ryme/Exception.hpp>
#include <Ryme/Log.hpp>
#include <Ryme/Ryme.hpp>

#include <SDL_vulkan.h>

#define VK_LAYER_KHRONOS_VALIDATION_NAME "VK_LAYER_KHRONOS_validation"

namespace ryme {

namespace Graphics {

SDL_Window * _sdlWindow = nullptr;

VkInstance _vkInstance = VK_NULL_HANDLE;

void initWindow(const InitInfo& initInfo);
void termWindow();
void initInstance(const InitInfo& initInfo);
void termInstance();

static VKAPI_ATTR VkBool32 VKAPI_CALL _VulkanDebugMessageCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT * callbackData,
    void * userData)
{
    Log("VkDebugUtilsMessenger", "{} {}",
        callbackData->pMessageIdName, 
        callbackData->pMessage);

    if (callbackData->objectCount > 0) {
        for (uint32_t i = 0; i < callbackData->objectCount; ++i) {
            const char * name = callbackData->pObjects[i].pObjectName;
            if (name) {
                Log("VkDebugUtilsMessenger", "\tObject #{}: Type {}, Value {:p}, Name '{}'",
                    i,
                    callbackData->pObjects[i].objectType,
                    callbackData->pObjects[i].objectHandle,
                    callbackData->pObjects[i].pObjectName);
            }
        }
    }

    if (callbackData->cmdBufLabelCount > 0) {
        for (uint32_t i = 0; i < callbackData->cmdBufLabelCount; ++i) {
            const char * name = callbackData->pCmdBufLabels[i].pLabelName;
            if (name) {
                Log("\tLabel #{}: {}", name);
            }
        }
    }

    return VK_FALSE;
}

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
    SDL_bool sdlResult;

    // Layers
    uint32_t availableLayerCount = 0;
    vkEnumerateInstanceLayerProperties(&availableLayerCount, nullptr);

    if (availableLayerCount == 0) {
        throw Exception("vkEnumerateInstanceLayerProperties() failed, no available layers");
    }

    List<VkLayerProperties> availableLayerList(availableLayerCount);
    vkResult = vkEnumerateInstanceLayerProperties(&availableLayerCount, availableLayerList.data());
    if (vkResult != VK_SUCCESS) {
        throw Exception("vkEnumerateInstanceLayerProperties() failed");
    }

    Map<String, VkLayerProperties> availableLayerMap;
    Log(RYME_ANCHOR, "Available Vulkan Layers:");
    for (const auto& layer : availableLayerList) {
        Log(RYME_ANCHOR, "\t{}: {}", layer.layerName, layer.description);
        availableLayerMap.emplace(layer.layerName, layer);
    }

    List<const char *> requiredLayerNameList = {

    };

    #if defined(RYME_BUILD_DEBUG)

        if (availableLayerMap.contains(VK_LAYER_KHRONOS_VALIDATION_NAME)) {
            requiredLayerNameList.push_back(VK_LAYER_KHRONOS_VALIDATION_NAME);
        }

    #endif

    Log(RYME_ANCHOR, "Required Vulkan Device Layers:");
    for (const auto& layer : requiredLayerNameList) {
        Log(RYME_ANCHOR, "\t{}", layer);
    }

    // Extensions
    uint32_t availableExtensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &availableExtensionCount, nullptr);

    if (availableExtensionCount == 0) {
        throw Exception("vkEnumerateInstanceExtensionProperties() failed, no extensions available.");
    }

    List<VkExtensionProperties> availableExtensionList(availableExtensionCount);
    vkResult = vkEnumerateInstanceExtensionProperties(nullptr, &availableExtensionCount, availableExtensionList.data());
    if (vkResult != VK_SUCCESS) {
        throw Exception("vkEnumerateInstanceExtensionProperties() failed.");
    }

    Map<String, VkExtensionProperties> availableExtensionMap;
    Log(RYME_ANCHOR, "Available Vulkan Instance Extensions:");
    for (const auto& extension : availableExtensionList) {
        Log(RYME_ANCHOR, "\t{}", extension.extensionName);
        availableExtensionMap.emplace(extension.extensionName, extension);
    }

    uint32_t requiredExtensionCount = 0;
    SDL_Vulkan_GetInstanceExtensions(_sdlWindow, &requiredExtensionCount, nullptr);

    List<const char *> requiredExtensionList(requiredExtensionCount);
    sdlResult = SDL_Vulkan_GetInstanceExtensions(_sdlWindow, &requiredExtensionCount, requiredExtensionList.data());
    if (!sdlResult) {
        throw Exception("SDL_Vulkan_GetInstanceExtensions() failed, {}", SDL_GetError());
    }

    #if defined(RYME_BUILD_DEBUG)

        if (availableExtensionMap.contains(VK_EXT_DEBUG_UTILS_EXTENSION_NAME)) {
            requiredExtensionList.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

    #endif

    Log(RYME_ANCHOR, "Required Vulkan Instance Extensions:");
    for (const auto& extension : requiredExtensionList) {
        Log(RYME_ANCHOR, "\t{}", extension);
    }

    // Instance
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
        .pEngineName = RYME_PROJECT_NAME,
        .engineVersion = VK_MAKE_VERSION(
            engineVersion.Major,
            engineVersion.Minor,
            engineVersion.Patch
        ),
        .apiVersion = VK_API_VERSION_1_2,
    };

    VkInstanceCreateInfo instanceCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .pApplicationInfo = &applicationInfo,
        .enabledLayerCount = static_cast<uint32_t>(requiredLayerNameList.size()),
        .ppEnabledLayerNames = requiredLayerNameList.data(),
        .enabledExtensionCount = static_cast<uint32_t>(requiredExtensionList.size()),
        .ppEnabledExtensionNames = requiredExtensionList.data(),
    };

    VkDebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfo;

    if (availableLayerMap.contains(VK_LAYER_KHRONOS_VALIDATION_NAME)) {
        VkDebugUtilsMessageSeverityFlagsEXT messageSeverity =
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;

        VkDebugUtilsMessageTypeFlagsEXT messageType =
            VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | 
            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | 
            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

        debugUtilsMessengerCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
            .pNext = nullptr,
            .flags = 0,
            .messageSeverity = messageSeverity,
            .messageType = messageType,
            .pfnUserCallback = _VulkanDebugMessageCallback,
            .pUserData = nullptr,
        };
        
        instanceCreateInfo.pNext = &debugUtilsMessengerCreateInfo;
    }

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