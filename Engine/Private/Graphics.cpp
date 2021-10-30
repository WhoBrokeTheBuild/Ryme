#include <Ryme/Graphics.hpp>
#include <Ryme/Exception.hpp>
#include <Ryme/Log.hpp>
#include <Ryme/Ryme.hpp>

#include <SDL_vulkan.h>

RYME_DISABLE_WARNINGS()

    #define VMA_IMPLEMENTATION
    #include <vk_mem_alloc.h>

RYME_ENABLE_WARNINGS()

#define VK_LAYER_KHRONOS_VALIDATION_NAME "VK_LAYER_KHRONOS_validation"

namespace ryme {

namespace Graphics {

SDL_Window * _sdlWindow = nullptr;

List<const char *> _vkRequiredLayerNameList;

VkInstance _vkInstance = VK_NULL_HANDLE;

VkSurfaceKHR _vkSurface = VK_NULL_HANDLE;

VkDebugUtilsMessengerEXT _vkDebugMessenger = VK_NULL_HANDLE;

VkPhysicalDeviceProperties _vkPhysicalDeviceProperties;

VkPhysicalDeviceFeatures _vkPhysicalDeviceFeatures;

VkPhysicalDevice _vkPhysicalDevice = VK_NULL_HANDLE;

VkDevice _vkDevice = VK_NULL_HANDLE;

Map<String, VkExtensionProperties> _vkAvailableDeviceExtensionMap;

uint32_t _vkGraphicsQueueFamilyIndex;

uint32_t _vkPresentQueueFamilyIndex;

VkQueue _vkGraphicsQueue = VK_NULL_HANDLE;

VkQueue _vkPresentQueue = VK_NULL_HANDLE;

VmaAllocator _vmaAllocator = VK_NULL_HANDLE;

void initWindow(const InitInfo& initInfo);
void termWindow();
void initInstance(const InitInfo& initInfo);
void termInstance();
void initSurface();
void termSurface();
void findPhysicalDevice();
void initDevice();
void termDevice();
void initAllocator();
void termAllocator();
void initSwapChain();
void termSwapChain();
void initSyncObjects();
void termSyncObjects();

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
    initSurface();
    initDevice();
    initAllocator();
}

RYME_API
void Term()
{
    termAllocator();
    termDevice();
    termSurface();
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

    Map<String, VkLayerProperties> _availableLayerMap;
    Log(RYME_ANCHOR, "Available Vulkan Layers:");
    for (const auto& layer : availableLayerList) {
        Log(RYME_ANCHOR, "\t{}: {}", layer.layerName, layer.description);
        _availableLayerMap.emplace(layer.layerName, layer);
    }

    _vkRequiredLayerNameList.clear();

    #if defined(RYME_BUILD_DEBUG)

        if (_availableLayerMap.contains(VK_LAYER_KHRONOS_VALIDATION_NAME)) {
            _vkRequiredLayerNameList.push_back(VK_LAYER_KHRONOS_VALIDATION_NAME);
        }

    #endif

    Log(RYME_ANCHOR, "Required Vulkan Device Layers:");
    for (const auto& layer : _vkRequiredLayerNameList) {
        Log(RYME_ANCHOR, "\t{}", layer);
    }

    // Extensions
    uint32_t availableExtensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &availableExtensionCount, nullptr);

    if (availableExtensionCount == 0) {
        throw Exception("vkEnumerateInstanceExtensionProperties() failed, no extensions available");
    }

    List<VkExtensionProperties> availableExtensionList(availableExtensionCount);
    vkResult = vkEnumerateInstanceExtensionProperties(nullptr, &availableExtensionCount, availableExtensionList.data());
    if (vkResult != VK_SUCCESS) {
        throw Exception("vkEnumerateInstanceExtensionProperties() failed");
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
        .apiVersion = VK_API_VERSION_1_1,
    };

    VkInstanceCreateInfo instanceCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .pApplicationInfo = &applicationInfo,
        .enabledLayerCount = static_cast<uint32_t>(_vkRequiredLayerNameList.size()),
        .ppEnabledLayerNames = _vkRequiredLayerNameList.data(),
        .enabledExtensionCount = static_cast<uint32_t>(requiredExtensionList.size()),
        .ppEnabledExtensionNames = requiredExtensionList.data(),
    };

    VkDebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfo;

    if (_availableLayerMap.contains(VK_LAYER_KHRONOS_VALIDATION_NAME)) {
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

void initSurface()
{
    SDL_bool sdlResult;

    sdlResult = SDL_Vulkan_CreateSurface(_sdlWindow, _vkInstance, &_vkSurface);
    if (!sdlResult) {
        throw Exception("SDL_Vulkan_CreateSurface() failed, {}", SDL_GetError());
    }
}

void termSurface()
{
    if (_vkSurface) {
        vkDestroySurfaceKHR(_vkInstance, _vkSurface, nullptr);
        _vkSurface = nullptr;
    }
}

void findPhysicalDevice()
{
    VkResult vkResult;

    uint32_t physicalDeviceCount = 0;
    vkEnumeratePhysicalDevices(_vkInstance, &physicalDeviceCount, nullptr);

    if (physicalDeviceCount == 0) {
        throw Exception("vkEnumeratePhysicalDevices() failed, no devices found");
    }

    List<VkPhysicalDevice> physicalDeviceList(physicalDeviceCount);
    vkResult = vkEnumeratePhysicalDevices(_vkInstance, &physicalDeviceCount, physicalDeviceList.data());
    if (vkResult != VK_SUCCESS) {
        throw Exception("vkEnumeratePhysicalDevices() failed");
    }

    for (const auto& physicalDevice : physicalDeviceList) {
        vkGetPhysicalDeviceProperties(physicalDevice, &_vkPhysicalDeviceProperties);
        vkGetPhysicalDeviceFeatures(physicalDevice, &_vkPhysicalDeviceFeatures);
        
        bool isSuitable = (
            _vkPhysicalDeviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
            _vkPhysicalDeviceFeatures.geometryShader
        );

        if (isSuitable) {
            _vkPhysicalDevice = physicalDevice;
            break;
        }
    }

    if (_vkPhysicalDevice == VK_NULL_HANDLE) {
        throw Exception("No suitable physical device found");
    }

    Log(RYME_ANCHOR, "Physical Device Name: {}", _vkPhysicalDeviceProperties.deviceName);

    Log(RYME_ANCHOR, "Physical Vulkan Version: {}.{}.{}",
        VK_VERSION_MAJOR(_vkPhysicalDeviceProperties.apiVersion),
        VK_VERSION_MINOR(_vkPhysicalDeviceProperties.apiVersion),
        VK_VERSION_PATCH(_vkPhysicalDeviceProperties.apiVersion));

    uint32_t availableExtensionCount = 0;
    vkEnumerateDeviceExtensionProperties(_vkPhysicalDevice, nullptr, &availableExtensionCount, nullptr);

    if (availableExtensionCount == 0) {
        throw Exception("vkEnumerateDeviceExtensionProperties() failed, no extensions available");
    }

    List<VkExtensionProperties> availableExtensions(availableExtensionCount);
    vkResult = vkEnumerateDeviceExtensionProperties(_vkPhysicalDevice, nullptr, &availableExtensionCount, availableExtensions.data());
    if (vkResult != VK_SUCCESS) {
        throw Exception("vkEnumerateDeviceExtensionProperties() failed");
    }

    Log(RYME_ANCHOR, "Available Vulkan Device Extensions:");
    for (const auto& extension : availableExtensions) {
        Log(RYME_ANCHOR, "\t{}", extension.extensionName);
        _vkAvailableDeviceExtensionMap.emplace(extension.extensionName, extension);
    }
}

void initDevice()
{
    VkResult vkResult;

    findPhysicalDevice();

    uint32_t queueFamilyCount;
    vkGetPhysicalDeviceQueueFamilyProperties(_vkPhysicalDevice, &queueFamilyCount, nullptr);
    if (queueFamilyCount == 0) {
        throw Exception("vkGetPhysicalDeviceQueueFamilyProperties(), no queues found");
    }

    List<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(_vkPhysicalDevice, &queueFamilyCount, queueFamilyProperties.data());

    _vkGraphicsQueueFamilyIndex = UINT32_MAX;
    _vkPresentQueueFamilyIndex = UINT32_MAX;

    Log(RYME_ANCHOR, "Available Vulkan Queue Families:");

    uint32_t index = 0;

    for (const auto& properties : queueFamilyProperties) {
        String types;

        if ((properties.queueFlags & VK_QUEUE_GRAPHICS_BIT) > 0) {
            types += "Graphics ";

            if (_vkGraphicsQueueFamilyIndex == UINT32_MAX) {
                _vkGraphicsQueueFamilyIndex = index;
                types.insert(types.end() - 1, '*');
            }
        }

        if ((properties.queueFlags & VK_QUEUE_COMPUTE_BIT) > 0) {
            types += "Compute ";
        }

        if ((properties.queueFlags & VK_QUEUE_TRANSFER_BIT) > 0) {
            types += "Transfer ";
        }

        if ((properties.queueFlags & VK_QUEUE_SPARSE_BINDING_BIT) > 0) {
            types += "SparseBinding ";
        }

        VkBool32 isPresentSupported = VK_FALSE;
        vkGetPhysicalDeviceSurfaceSupportKHR(
            _vkPhysicalDevice,
            index,
            _vkSurface,
            &isPresentSupported
        );

        if (isPresentSupported) {
            types += "Present ";

            if (_vkPresentQueueFamilyIndex == UINT32_MAX) {
                _vkPresentQueueFamilyIndex = index;
                types.insert(types.end() - 1, '*');
            }
        }

        Log(RYME_ANCHOR, "Queue #{}: {}", properties.queueCount, types);
        ++index;
    }

    if (_vkGraphicsQueueFamilyIndex == UINT32_MAX) {
        throw Exception("No suitable graphics queue found");
    }

    if (_vkPresentQueueFamilyIndex == UINT32_MAX) {
        throw Exception("No suitable graphics queue found");
    }

    const float queuePriorities = 1.0f;

    List<VkDeviceQueueCreateInfo> queueCreateInfoList;
    Set<uint32_t> queueFamilyIndexSet = {
        _vkGraphicsQueueFamilyIndex,
        _vkPresentQueueFamilyIndex
    };

    for (auto index : queueFamilyIndexSet) {
        queueCreateInfoList.push_back(
            VkDeviceQueueCreateInfo{
                .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .queueFamilyIndex = index,
                .queueCount = 1,
                .pQueuePriorities = &queuePriorities,
            }
        );
    }

    VkPhysicalDeviceFeatures requiredDeviceFeatures = {
        // TODO
    };

    uint32_t availableExtensionCount = 0;
    vkEnumerateDeviceExtensionProperties(
        _vkPhysicalDevice,
        nullptr,
        &availableExtensionCount,
        nullptr
    );

    if (availableExtensionCount == 0) {
        throw Exception("vkEnumerateDeviceExtensionProperties() failed, no extensions found");
    }

    List<VkExtensionProperties> availableExtensionList(availableExtensionCount);
    vkResult = vkEnumerateDeviceExtensionProperties(
        _vkPhysicalDevice,
        nullptr,
        &availableExtensionCount,
        availableExtensionList.data()
    );

    if (vkResult != VK_SUCCESS) {
        throw Exception("vkEnumerateDeviceExtensionProperties(), failed");
    }

    Log(RYME_ANCHOR, "Available Vulkan Device Extensions:");
    for (const auto& extension : availableExtensionList) {
        Log(RYME_ANCHOR, "\t{}", extension.extensionName);
        _vkAvailableDeviceExtensionMap.emplace(extension.extensionName, extension);
    }

    List<const char *> requiredExtensionNameList = { };
    
    if (_vkAvailableDeviceExtensionMap.contains(VK_EXT_MEMORY_BUDGET_EXTENSION_NAME)) {
        requiredExtensionNameList.push_back(VK_EXT_MEMORY_BUDGET_EXTENSION_NAME);
    }

    requiredExtensionNameList.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

    Log(RYME_ANCHOR, "Required Vulkan Device Extensions:");
    for (auto extension : requiredExtensionNameList) {
        Log(RYME_ANCHOR, "\t{}", extension);
    }

    // _vkRequiredLayerNameList
    VkDeviceCreateInfo deviceCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfoList.size()),
        .pQueueCreateInfos = queueCreateInfoList.data(),
        .enabledLayerCount = static_cast<uint32_t>(_vkRequiredLayerNameList.size()),
        .ppEnabledLayerNames = _vkRequiredLayerNameList.data(),
        .enabledExtensionCount = static_cast<uint32_t>(requiredExtensionNameList.size()),
        .ppEnabledExtensionNames = requiredExtensionNameList.data(),
        .pEnabledFeatures = &requiredDeviceFeatures,
    };

    vkResult = vkCreateDevice(_vkPhysicalDevice, &deviceCreateInfo, nullptr, &_vkDevice);
    if (vkResult != VK_SUCCESS) {
        throw Exception("vkCreateDeivce() failed");
    }

    vkGetDeviceQueue(_vkDevice, _vkGraphicsQueueFamilyIndex, 0, &_vkGraphicsQueue);
    vkGetDeviceQueue(_vkDevice, _vkPresentQueueFamilyIndex, 0, & _vkPresentQueue);
}

void termDevice()
{
    if (_vkDevice) {
        vkDestroyDevice(_vkDevice, nullptr);
        _vkDevice = nullptr;
    }
}

void initAllocator()
{
    VkResult vkResult;

    VmaAllocatorCreateFlags allocatorCreateFlags = 0;

    if (_vkAvailableDeviceExtensionMap.contains(VK_EXT_MEMORY_BUDGET_EXTENSION_NAME)) {
        allocatorCreateFlags |= VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT;
    }

    VmaAllocatorCreateInfo allocatorCreateInfo = {
        .flags = allocatorCreateFlags,
        .physicalDevice = _vkPhysicalDevice,
        .device = _vkDevice,
        .preferredLargeHeapBlockSize = 0,
        .pAllocationCallbacks = nullptr,
        .pDeviceMemoryCallbacks = nullptr,
        .instance = _vkInstance,
        .vulkanApiVersion = VK_API_VERSION_1_1,
    };

    vkResult = vmaCreateAllocator(&allocatorCreateInfo, &_vmaAllocator);
    if (vkResult != VK_SUCCESS) {
        throw Exception("vmaCreateAllocator() failed");
    }
}

void termAllocator()
{
    if (_vmaAllocator) {
        vmaDestroyAllocator(_vmaAllocator);
        _vmaAllocator = nullptr;
    }
}

void initSwapChain()
{

}

void termSwapChain()
{

}

void initSyncObjects()
{

}

void termSyncObjects()
{

}

} // namespace Graphics

} // namespace ryme