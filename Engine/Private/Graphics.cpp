#include <Ryme/Graphics.hpp>
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

List<VkSemaphore> _vkImageAvailableSemaphoreList;

List<VkSemaphore> _vkRenderingFinishedSemaphoreList;

List<VkFence> _vkInFlightFenceList;

List<VkFence> _vkImageInFlightList;

uint32_t _backbufferCount = 2;

VkExtent2D _vkSwapChainExtent;

Vec2i _windowSize;

VkSwapchainKHR _vkSwapChain = VK_NULL_HANDLE;

VkFormat _vkSwapChainImageFormat;

List<VkImage> _vkSwapChainImageList;

List<VkImageView> _vkSwapChainImageViewList;

VkFormat _vkDepthImageFormat;

VkImage _vkDepthImage = VK_NULL_HANDLE;

VmaAllocation _vmaDepthImageAllocation = VK_NULL_HANDLE;

VkImageView _vkDepthImageView = VK_NULL_HANDLE;

VkRenderPass _vkRenderPass = VK_NULL_HANDLE;

VkDescriptorPool _vkDescriptorPool = VK_NULL_HANDLE;

List<VkDescriptorSetLayout> _vkDescriptorSetLayoutList;

void initWindow(String windowTitle);
void termWindow();
void initInstance();
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
void initRenderPass();
void termRenderPass();
void initDescriptorPool();
void termDescriptorPool();

void initDepthBuffer();
void termDepthBuffer();

String VkResultToString(VkResult vkResult);
String VkFormatToString(VkFormat vkFormat);
String VkColorSpaceToString(VkColorSpaceKHR vkColorSpace);
String VkPresentModeToString(VkPresentModeKHR vkPresentMode);

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
void Init(String windowTitle, Vec2i windowSize)
{
    _windowSize = windowSize;
    
    initWindow(windowTitle);
    initInstance();
    initSurface();
    initDevice();
    initAllocator();
    initSwapChain();
    initSyncObjects();
}

RYME_API
void Term()
{
    termSyncObjects();
    termSwapChain();
    termAllocator();
    termDevice();
    termSurface();
    termInstance();
    termWindow();
}

void initWindow(String windowTitle)
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
        windowTitle.c_str(),
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        _windowSize.x,
        _windowSize.y,
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

void initInstance()
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
    const auto& applicationVersion = GetApplicationVersion();

    VkApplicationInfo applicationInfo = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pNext = nullptr,
        .pApplicationName = GetApplicationName().c_str(),
        .applicationVersion = VK_MAKE_VERSION(
            applicationVersion.Major,
            applicationVersion.Minor,
            applicationVersion.Patch
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
    VkResult vkResult;

    uint32_t formatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(
        _vkPhysicalDevice,
        _vkSurface,
        &formatCount,
        nullptr
    );

    if (formatCount == 0) {
        throw Exception("vkGetPhysicalDeviceSurfaceFormatsKHR() failed, no surface formats found");
    }

    List<VkSurfaceFormatKHR> formatList(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(
        _vkPhysicalDevice,
        _vkSurface,
        &formatCount,
        formatList.data()
    );

    Log(RYME_ANCHOR, "Available Vulkan Surface Formats:");
    VkSurfaceFormatKHR surfaceFormat = formatList[0];
    for (const auto& format : formatList) {
        bool isSRGB = (
            format.format == VK_FORMAT_R8G8B8A8_SRGB ||
            format.format == VK_FORMAT_B8G8R8A8_SRGB
        );

        if (isSRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            surfaceFormat = format;
        }

        Log(RYME_ANCHOR, "\t{} {}",
            VkFormatToString(format.format),
            VkColorSpaceToString(format.colorSpace));
    }

    Log(RYME_ANCHOR, "Vulkan Swap Chain Image Format: {}",
        VkFormatToString(surfaceFormat.format));

    Log(RYME_ANCHOR, "Vuilkan Swap Chain Image Color Space: {}",
        VkColorSpaceToString(surfaceFormat.colorSpace));

    uint32_t presentModeCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(
        _vkPhysicalDevice,
        _vkSurface,
        &presentModeCount,
        nullptr
    );

    if (presentModeCount == 0) {
        throw Exception("vkGetPhysicalDeviceSurfacePresentModeKHR() failed, no present modes found");
    }

    List<VkPresentModeKHR> presentModeList(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(
        _vkPhysicalDevice,
        _vkSurface,
        &presentModeCount,
        presentModeList.data()
    );

    // VK_PRESENT_MODE_IMMEDIATE_KHR = Do not wait for vsync, may cause screen tearing
    // VK_PRESENT_MODE_FIFO_KHR = Queue of presentation requests, wait for vsync, required to be supported
    //    equivalent to {wgl|glX|egl}SwapBuffers(1)
    // VK_PRESENT_MODE_FIFO_RELAXED_KHR = Similar to FIFO, but will not wait for a second vsync period 
    //    if the first has already passed, may cause screen tearing
    //    equivalent to {wgl|glX}SwapBuffers(-1)
    // VK_PRESENT_MODE_MAILBOX_KHR = Queue of presentation requests, wait for vsync, replaces entries if the queue is full

    Log(RYME_ANCHOR, "Available Vulkan Present Modes:");

    // FIFO is the only present mode required to be supported
    VkPresentModeKHR swapChainPresentMode = VK_PRESENT_MODE_FIFO_KHR;
    for (const auto& presentMode : presentModeList) {
        if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            swapChainPresentMode = presentMode;
        }

        Log(RYME_ANCHOR, "\t{}", VkPresentModeToString(presentMode));
    }

    Log(RYME_ANCHOR, "Vulkan Swap Chain Present Mode: {}",
        VkPresentModeToString(swapChainPresentMode));

    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
        _vkPhysicalDevice,
        _vkSurface,
        &surfaceCapabilities
    );

    _vkSwapChainExtent = surfaceCapabilities.currentExtent;

    if (_vkSwapChainExtent.width == UINT32_MAX || _vkSwapChainExtent.height == UINT32_MAX) {
        const Vec2i& size = _windowSize;

        _vkSwapChainExtent.width = std::clamp(
            static_cast<uint32_t>(size.x),
            surfaceCapabilities.minImageExtent.width,
            surfaceCapabilities.maxImageExtent.width
        );

        _vkSwapChainExtent.height = std::clamp(
            static_cast<uint32_t>(size.y),
            surfaceCapabilities.minImageExtent.height,
            surfaceCapabilities.maxImageExtent.height
        );
    }

    Log(RYME_ANCHOR, "Vulkan Swap Chain Extent: {}x{}",
        _vkSwapChainExtent.width,
        _vkSwapChainExtent.height
    );

    uint32_t imageCount = std::clamp(
        (unsigned)_backbufferCount,
        surfaceCapabilities.minImageCount,
        surfaceCapabilities.maxImageCount
    );

    Log(RYME_ANCHOR, "Vulakn Swap Chain Image Count: {}", imageCount);

    VkSwapchainKHR oldSwapChain = _vkSwapChain;

    uint32_t queueFamilyIndices[] = {
        _vkGraphicsQueueFamilyIndex,
        _vkPresentQueueFamilyIndex,
    };

    VkSharingMode sharingMode = (
        _vkGraphicsQueueFamilyIndex == _vkPresentQueueFamilyIndex
        ? VK_SHARING_MODE_EXCLUSIVE
        : VK_SHARING_MODE_CONCURRENT
    );

    VkSwapchainCreateInfoKHR swapChainCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .pNext = nullptr,
        .flags = 0,
        .surface = _vkSurface,
        .minImageCount = imageCount,
        .imageFormat = surfaceFormat.format,
        .imageColorSpace = surfaceFormat.colorSpace,
        .imageExtent = _vkSwapChainExtent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .imageSharingMode = sharingMode,
        .queueFamilyIndexCount = 2,
        .pQueueFamilyIndices = queueFamilyIndices,
        .preTransform = surfaceCapabilities.currentTransform,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = swapChainPresentMode,
        .clipped = VK_TRUE,
        .oldSwapchain = oldSwapChain,
    };

    vkResult = vkCreateSwapchainKHR(
        _vkDevice,
        &swapChainCreateInfo,
        nullptr,
        &_vkSwapChain
    );

    if (vkResult != VK_SUCCESS) {
        throw Exception("vkCreateSwapchainKHR() failed");
    }

    if (oldSwapChain) {
        vkDestroySwapchainKHR(_vkDevice, oldSwapChain, nullptr);
        oldSwapChain = VK_NULL_HANDLE;
    }

    _vkSwapChainImageFormat = surfaceFormat.format;

    vkGetSwapchainImagesKHR(
        _vkDevice,
        _vkSwapChain,
        &imageCount,
        nullptr
    );

    _vkSwapChainImageList.resize(imageCount, VK_NULL_HANDLE);

    vkGetSwapchainImagesKHR(
        _vkDevice,
        _vkSwapChain,
        &imageCount,
        _vkSwapChainImageList.data()
    );

    _vkSwapChainImageViewList.resize(imageCount, VK_NULL_HANDLE);

    for (unsigned i = 0; i < _backbufferCount; ++i) {
        if (_vkSwapChainImageViewList[i]) {
            vkDestroyImageView(_vkDevice, _vkSwapChainImageViewList[i], nullptr);
            _vkSwapChainImageViewList[i] = VK_NULL_HANDLE;
        }

        VkImageViewCreateInfo imageViewCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .image = _vkSwapChainImageList[i],
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = _vkSwapChainImageFormat,
            .subresourceRange = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
        };

        vkResult = vkCreateImageView(_vkDevice, &imageViewCreateInfo, nullptr, &_vkSwapChainImageViewList[i]);
        if (vkResult != VK_SUCCESS) {
            throw Exception("vkCreateImageView() failed for image view #{}", i);
        }

        _backbufferCount = imageCount;

        initDepthBuffer();
        initRenderPass();
        // initDescriptorPool();
        // initPipelineLayout();
        // initFramebuffers();
        // initCommandBuffers();
    }
}

void termSwapChain()
{
    // termCommandBuffers();
    // termFramebuffers();
    // termPipelineLayout();
    // termDescriptorPool();
    termRenderPass();
    termDepthBuffer();

    for (auto& imageView : _vkSwapChainImageViewList) {
        if (imageView) {
            vkDestroyImageView(_vkDevice, imageView, nullptr);
            imageView = nullptr;
        }
    }

    if (_vkSwapChain) {
        vkDestroySwapchainKHR(_vkDevice, _vkSwapChain, nullptr);
        _vkSwapChain = VK_NULL_HANDLE;
    }
}

void resetSwapChain()
{
    if (_vkSwapChain) {
        vkDeviceWaitIdle(_vkDevice);

        initSwapChain();
    }
}

void initSyncObjects()
{
    VkResult vkResult;

    _vkImageAvailableSemaphoreList.resize(_backbufferCount, VK_NULL_HANDLE);
    _vkRenderingFinishedSemaphoreList.resize(_backbufferCount, VK_NULL_HANDLE);
    _vkInFlightFenceList.resize(_backbufferCount, VK_NULL_HANDLE);
    _vkImageInFlightList.resize(_backbufferCount, VK_NULL_HANDLE);

    VkSemaphoreCreateInfo semaphoreCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
    };

    VkFenceCreateInfo fenceCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .pNext = nullptr,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT,
    };

    for (unsigned i = 0; i < _backbufferCount; ++i) {
        vkResult = vkCreateSemaphore(
            _vkDevice,
            &semaphoreCreateInfo,
            nullptr,
            &_vkImageAvailableSemaphoreList[i]
        );

        if (vkResult != VK_SUCCESS) {
            throw Exception("vkCreateSemaphore() failed");
        }   

        vkResult = vkCreateSemaphore(
            _vkDevice,
            &semaphoreCreateInfo,
            nullptr,
            &_vkRenderingFinishedSemaphoreList[i]
        );

        if (vkResult != VK_SUCCESS) {
            throw Exception("vkCreateSemaphore() failed");
        }

        vkResult = vkCreateFence(
            _vkDevice,
            &fenceCreateInfo,
            nullptr,
            &_vkInFlightFenceList[i]
        );

        if (vkResult != VK_SUCCESS) {
            throw Exception("vkCreateFence() failed");
        }
    }
}

void termSyncObjects()
{
    for (auto& fence : _vkImageInFlightList) {
        // TODO: This could blow up be careful.
        vkDestroyFence(_vkDevice, fence, VK_NULL_HANDLE);
        fence = VK_NULL_HANDLE;
    }

    for (auto& fence : _vkInFlightFenceList) {
        vkDestroyFence(_vkDevice, fence, VK_NULL_HANDLE);
        fence = VK_NULL_HANDLE;
    }

    for (auto& semaphore : _vkRenderingFinishedSemaphoreList) {
        vkDestroySemaphore(_vkDevice, semaphore, VK_NULL_HANDLE);
        semaphore = VK_NULL_HANDLE;
    }

    for (auto& semaphore : _vkImageAvailableSemaphoreList) {
        vkDestroySemaphore(_vkDevice, semaphore, VK_NULL_HANDLE);
        semaphore = VK_NULL_HANDLE;
    }
}

void initDepthBuffer()
{
    VkResult vkResult;

    termDepthBuffer();

    // TODO: Investigate
    List<VkFormat> potentialFormatList = {
        VK_FORMAT_D32_SFLOAT,
        VK_FORMAT_D32_SFLOAT_S8_UINT,
        VK_FORMAT_D24_UNORM_S8_UINT,
        VK_FORMAT_D16_UNORM,
        VK_FORMAT_D16_UNORM_S8_UINT,
    };

    _vkDepthImageFormat = VK_FORMAT_UNDEFINED;

    for (auto format : potentialFormatList) {
        VkFormatProperties formatProperties;
        vkGetPhysicalDeviceFormatProperties(_vkPhysicalDevice, format, &formatProperties);

        bool isSuitable = (
            (formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) > 0
        );

        if (isSuitable) {
            _vkDepthImageFormat = format;
            break;
        }
    }

    if (_vkDepthImageFormat == VK_FORMAT_UNDEFINED) {
        throw Exception("Unable to find suitable depth buffer image format");
    }

    Log(RYME_ANCHOR, "Vulkan Depth Buffer Image Format: {}",
        VkFormatToString(_vkDepthImageFormat));

    VkImageCreateInfo imageCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = _vkDepthImageFormat,
        .extent = {
            .width = _vkSwapChainExtent.width,
            .height = _vkSwapChainExtent.height,
            .depth = 1,
        },
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    };

    VmaAllocationCreateInfo allocationCreateInfo = {
        .flags = 0,
        .usage = VMA_MEMORY_USAGE_GPU_ONLY,
    };

    vkResult = vmaCreateImage(
        _vmaAllocator,
        &imageCreateInfo,
        &allocationCreateInfo,
        &_vkDepthImage,
        &_vmaDepthImageAllocation,
        nullptr
    );

    if (vkResult != VK_SUCCESS) {
        throw Exception("vmaCreateImage() failed, unable to create depth buffer image");
    }

    VkImageViewCreateInfo imageViewCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .image = _vkDepthImage,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = _vkDepthImageFormat,
        .subresourceRange = {
            .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
        },
    };

    vkResult = vkCreateImageView(
        _vkDevice,
        &imageViewCreateInfo,
        nullptr,
        &_vkDepthImageView
    );

    if (vkResult != VK_SUCCESS) {
        throw Exception("vkCreateImageView failed, unable to create depth buffer image view");
    }
}

void termDepthBuffer()
{
    if (_vkDepthImage) {
        vkDestroyImage(_vkDevice, _vkDepthImage, nullptr);
        _vkDepthImage = VK_NULL_HANDLE;
    }

    if (_vmaDepthImageAllocation) {
        vmaFreeMemory(_vmaAllocator, _vmaDepthImageAllocation);
        _vmaDepthImageAllocation = VK_NULL_HANDLE;
    }

    if (_vkDepthImageView) {
        vkDestroyImageView(_vkDevice, _vkDepthImageView, nullptr);
        _vkDepthImageView = VK_NULL_HANDLE;
    }
}

void initRenderPass()
{
    VkResult vkResult;

    termRenderPass();

    VkAttachmentDescription colorAttachmentDescription = {
        .flags = 0,
        .format = _vkSwapChainImageFormat,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
    };

    VkAttachmentDescription depthAttachmentDescription = {
        .flags = 0,
        .format = _vkDepthImageFormat,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
    };

    Array<VkAttachmentDescription, 2> attachmentList = {
        colorAttachmentDescription,
        depthAttachmentDescription,
    };

    VkAttachmentReference colorAttachmentReference = {
        .attachment = 0,
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    };

    VkAttachmentReference depthAttachmentReference = {
        .attachment = 1,
        .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
    };

    Array<VkSubpassDescription, 1> subpassDescriptionList = {
        VkSubpassDescription {
            .flags = 0,
            .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
            .inputAttachmentCount = 0,
            .pInputAttachments = nullptr,
            .colorAttachmentCount = 1,
            .pColorAttachments = &colorAttachmentReference,
            .pResolveAttachments = nullptr,
            .pDepthStencilAttachment = &depthAttachmentReference,
            .preserveAttachmentCount = 0,
            .pPreserveAttachments = nullptr,
        },
    };

    Array<VkSubpassDependency, 1> subpassDependencyList = {
        VkSubpassDependency {
            .srcSubpass = VK_SUBPASS_EXTERNAL,
            .dstSubpass = 0,
            .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
            .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
            .srcAccessMask = 0,
            .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
        },
    };

    VkRenderPassCreateInfo renderPassCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .attachmentCount = static_cast<uint32_t>(attachmentList.size()),
        .pAttachments = attachmentList.data(),
        .subpassCount = static_cast<uint32_t>(subpassDescriptionList.size()),
        .pSubpasses = subpassDescriptionList.data(),
        .dependencyCount = static_cast<uint32_t>(subpassDependencyList.size()),
        .pDependencies = subpassDependencyList.data(),
    };

    vkResult = vkCreateRenderPass(_vkDevice, &renderPassCreateInfo, nullptr, &_vkRenderPass);

    if (vkResult != VK_SUCCESS) {
        throw Exception("vkCreateRenderPass() failed");
    }
}

void termRenderPass()
{
    if (_vkRenderPass) {
        vkDestroyRenderPass(_vkDevice, _vkRenderPass, nullptr);
        _vkRenderPass = VK_NULL_HANDLE;
    }
}

void initDescriptorPool()
{
    VkResult vkResult;

    termDescriptorPool();

    Array<VkDescriptorPoolSize, 1> descriptorPoolSizeList = {
        VkDescriptorPoolSize {
            .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = static_cast<uint32_t>(1), // materialList.size() + meshList.size(), can never be 0
        },
    };

    VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .maxSets = _backbufferCount,
        .poolSizeCount = static_cast<uint32_t>(descriptorPoolSizeList.size()),
        .pPoolSizes = descriptorPoolSizeList.data(),
    };

    vkResult = vkCreateDescriptorPool(
        _vkDevice,
        &descriptorPoolCreateInfo,
        nullptr,
        &_vkDescriptorPool
    );

    if (vkResult != VK_SUCCESS) {
        throw Exception("vkCreateDescriptorPool() failed");
    }

    Array<VkDescriptorSetLayoutBinding, 2> descriptorSetLayoutBindingList = {
        VkDescriptorSetLayoutBinding {
            .binding = 0U, // TODO: ShaderGlobals::Binding,
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS,
            .pImmutableSamplers = nullptr,
        },
        VkDescriptorSetLayoutBinding {
            .binding = 1U, //TODO: ShaderTransform::Binding,
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
            .pImmutableSamplers = nullptr,
        },
        // TODO: ShaderMaterial
    };

    _vkDescriptorSetLayoutList.resize(1, VK_NULL_HANDLE);

    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .bindingCount = static_cast<uint32_t>(descriptorSetLayoutBindingList.size()),
        .pBindings = descriptorSetLayoutBindingList.data(),
    };

    vkResult = vkCreateDescriptorSetLayout(
        _vkDevice,
        &descriptorSetLayoutCreateInfo,
        nullptr,
        &_vkDescriptorSetLayoutList[0]
    );

    if (vkResult != VK_SUCCESS) {
        throw Exception("vkCreateDescriptorSetLayout() failed");
    }
}

void termDescriptorPool()
{
    for (auto& descriptorSetLayout : _vkDescriptorSetLayoutList) {
        if (descriptorSetLayout) {
            vkDestroyDescriptorSetLayout(_vkDevice, descriptorSetLayout, nullptr);
            descriptorSetLayout = VK_NULL_HANDLE;
        }
    }

    if (_vkDescriptorPool) {
        vkDestroyDescriptorPool(_vkDevice, _vkDescriptorPool, nullptr);
        _vkDescriptorPool = VK_NULL_HANDLE;
    }
}

String VkResultToString(VkResult vkResult)
{
    switch (vkResult) {
        case VK_SUCCESS:
            return "VK_SUCCESS";
        case VK_NOT_READY:
            return "VK_NOT_READY";
        case VK_TIMEOUT:
            return "VK_TIMEOUT";
        case VK_EVENT_SET:
            return "VK_EVENT_SET";
        case VK_EVENT_RESET:
            return "VK_EVENT_RESET";
        case VK_INCOMPLETE:
            return "VK_INCOMPLETE";
        case VK_ERROR_OUT_OF_HOST_MEMORY:
            return "VK_ERROR_OUT_OF_HOST_MEMORY";
        case VK_ERROR_OUT_OF_DEVICE_MEMORY:
            return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
        case VK_ERROR_INITIALIZATION_FAILED:
            return "VK_ERROR_INITIALIZATION_FAILED";
        case VK_ERROR_DEVICE_LOST:
            return "VK_ERROR_DEVICE_LOST";
        case VK_ERROR_MEMORY_MAP_FAILED:
            return "VK_ERROR_MEMORY_MAP_FAILED";
        case VK_ERROR_LAYER_NOT_PRESENT:
            return "VK_ERROR_LAYER_NOT_PRESENT";
        case VK_ERROR_EXTENSION_NOT_PRESENT:
            return "VK_ERROR_EXTENSION_NOT_PRESENT";
        case VK_ERROR_FEATURE_NOT_PRESENT:
            return "VK_ERROR_FEATURE_NOT_PRESENT";
        case VK_ERROR_INCOMPATIBLE_DRIVER:
            return "VK_ERROR_INCOMPATIBLE_DRIVER";
        case VK_ERROR_TOO_MANY_OBJECTS:
            return "VK_ERROR_TOO_MANY_OBJECTS";
        case VK_ERROR_FORMAT_NOT_SUPPORTED:
            return "VK_ERROR_FORMAT_NOT_SUPPORTED";
        case VK_ERROR_FRAGMENTED_POOL:
            return "VK_ERROR_FRAGMENTED_POOL";
        case VK_ERROR_UNKNOWN:
            return "VK_ERROR_UNKNOWN";
        case VK_ERROR_OUT_OF_POOL_MEMORY:
            return "VK_ERROR_OUT_OF_POOL_MEMORY";
        case VK_ERROR_INVALID_EXTERNAL_HANDLE:
            return "VK_ERROR_INVALID_EXTERNAL_HANDLE";
        case VK_ERROR_FRAGMENTATION:
            return "VK_ERROR_FRAGMENTATION";
        case VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS:
            return "VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS";
        default:
            return fmt::format("Unknown ({})", vkResult);
    }
}

String VkFormatToString(VkFormat vkFormat)
{
    // Note: All formats that do not include RGBA have been removed for brevity
    switch (vkFormat) {
        case VK_FORMAT_UNDEFINED:
            return "VK_FORMAT_UNDEFINED";
        case VK_FORMAT_R8G8B8A8_UNORM:
            return "VK_FORMAT_R8G8B8A8_UNORM";
        case VK_FORMAT_R8G8B8A8_SNORM:
            return "VK_FORMAT_R8G8B8A8_SNORM";
        case VK_FORMAT_R8G8B8A8_USCALED:
            return "VK_FORMAT_R8G8B8A8_USCALED";
        case VK_FORMAT_R8G8B8A8_SSCALED:
            return "VK_FORMAT_R8G8B8A8_SSCALED";
        case VK_FORMAT_R8G8B8A8_UINT:
            return "VK_FORMAT_R8G8B8A8_UINT";
        case VK_FORMAT_R8G8B8A8_SINT:
            return "VK_FORMAT_R8G8B8A8_SINT";
        case VK_FORMAT_R8G8B8A8_SRGB:
            return "VK_FORMAT_R8G8B8A8_SRGB";
        case VK_FORMAT_B8G8R8A8_UNORM:
            return "VK_FORMAT_B8G8R8A8_UNORM";
        case VK_FORMAT_B8G8R8A8_SNORM:
            return "VK_FORMAT_B8G8R8A8_SNORM";
        case VK_FORMAT_B8G8R8A8_USCALED:
            return "VK_FORMAT_B8G8R8A8_USCALED";
        case VK_FORMAT_B8G8R8A8_SSCALED:
            return "VK_FORMAT_B8G8R8A8_SSCALED";
        case VK_FORMAT_B8G8R8A8_UINT:
            return "VK_FORMAT_B8G8R8A8_UINT";
        case VK_FORMAT_B8G8R8A8_SINT:
            return "VK_FORMAT_B8G8R8A8_SINT";
        case VK_FORMAT_B8G8R8A8_SRGB:
            return "VK_FORMAT_B8G8R8A8_SRGB";
        case VK_FORMAT_A8B8G8R8_UNORM_PACK32:
            return "VK_FORMAT_A8B8G8R8_UNORM_PACK32";
        case VK_FORMAT_A8B8G8R8_SNORM_PACK32:
            return "VK_FORMAT_A8B8G8R8_SNORM_PACK32";
        case VK_FORMAT_A8B8G8R8_USCALED_PACK32:
            return "VK_FORMAT_A8B8G8R8_USCALED_PACK32";
        case VK_FORMAT_A8B8G8R8_SSCALED_PACK32:
            return "VK_FORMAT_A8B8G8R8_SSCALED_PACK32";
        case VK_FORMAT_A8B8G8R8_UINT_PACK32:
            return "VK_FORMAT_A8B8G8R8_UINT_PACK32";
        case VK_FORMAT_A8B8G8R8_SINT_PACK32:
            return "VK_FORMAT_A8B8G8R8_SINT_PACK32";
        case VK_FORMAT_A8B8G8R8_SRGB_PACK32:
            return "VK_FORMAT_A8B8G8R8_SRGB_PACK32";
        case VK_FORMAT_A2R10G10B10_UNORM_PACK32:
            return "VK_FORMAT_A2R10G10B10_UNORM_PACK32";
        case VK_FORMAT_A2R10G10B10_SNORM_PACK32:
            return "VK_FORMAT_A2R10G10B10_SNORM_PACK32";
        case VK_FORMAT_A2R10G10B10_USCALED_PACK32:
            return "VK_FORMAT_A2R10G10B10_USCALED_PACK32";
        case VK_FORMAT_A2R10G10B10_SSCALED_PACK32:
            return "VK_FORMAT_A2R10G10B10_SSCALED_PACK32";
        case VK_FORMAT_A2R10G10B10_UINT_PACK32:
            return "VK_FORMAT_A2R10G10B10_UINT_PACK32";
        case VK_FORMAT_A2R10G10B10_SINT_PACK32:
            return "VK_FORMAT_A2R10G10B10_SINT_PACK32";
        case VK_FORMAT_A2B10G10R10_UNORM_PACK32:
            return "VK_FORMAT_A2B10G10R10_UNORM_PACK32";
        case VK_FORMAT_A2B10G10R10_SNORM_PACK32:
            return "VK_FORMAT_A2B10G10R10_SNORM_PACK32";
        case VK_FORMAT_A2B10G10R10_USCALED_PACK32:
            return "VK_FORMAT_A2B10G10R10_USCALED_PACK32";
        case VK_FORMAT_A2B10G10R10_SSCALED_PACK32:
            return "VK_FORMAT_A2B10G10R10_SSCALED_PACK32";
        case VK_FORMAT_A2B10G10R10_UINT_PACK32:
            return "VK_FORMAT_A2B10G10R10_UINT_PACK32";
        case VK_FORMAT_A2B10G10R10_SINT_PACK32:
            return "VK_FORMAT_A2B10G10R10_SINT_PACK32";
        case VK_FORMAT_R16G16B16A16_UNORM:
            return "VK_FORMAT_R16G16B16A16_UNORM";
        case VK_FORMAT_R16G16B16A16_SNORM:
            return "VK_FORMAT_R16G16B16A16_SNORM";
        case VK_FORMAT_R16G16B16A16_USCALED:
            return "VK_FORMAT_R16G16B16A16_USCALED";
        case VK_FORMAT_R16G16B16A16_SSCALED:
            return "VK_FORMAT_R16G16B16A16_SSCALED";
        case VK_FORMAT_R16G16B16A16_UINT:
            return "VK_FORMAT_R16G16B16A16_UINT";
        case VK_FORMAT_R16G16B16A16_SINT:
            return "VK_FORMAT_R16G16B16A16_SINT";
        case VK_FORMAT_R16G16B16A16_SFLOAT:
            return "VK_FORMAT_R16G16B16A16_SFLOAT";
        case VK_FORMAT_R32G32B32A32_UINT:
            return "VK_FORMAT_R32G32B32A32_UINT";
        case VK_FORMAT_R32G32B32A32_SINT:
            return "VK_FORMAT_R32G32B32A32_SINT";
        case VK_FORMAT_R32G32B32A32_SFLOAT:
            return "VK_FORMAT_R32G32B32A32_SFLOAT";
        case VK_FORMAT_R64G64B64A64_UINT:
            return "VK_FORMAT_R64G64B64A64_UINT";
        case VK_FORMAT_R64G64B64A64_SINT:
            return "VK_FORMAT_R64G64B64A64_SINT";
        case VK_FORMAT_R64G64B64A64_SFLOAT:
            return "VK_FORMAT_R64G64B64A64_SFLOAT";
        case VK_FORMAT_D16_UNORM:
            return "VK_FORMAT_D16_UNORM";
        case VK_FORMAT_D32_SFLOAT:
            return "VK_FORMAT_D32_SFLOAT";
        case VK_FORMAT_D16_UNORM_S8_UINT:
            return "VK_FORMAT_D16_UNORM_S8_UINT";
        case VK_FORMAT_D24_UNORM_S8_UINT:
            return "VK_FORMAT_D24_UNORM_S8_UINT";
        case VK_FORMAT_D32_SFLOAT_S8_UINT:
            return "VK_FORMAT_D32_SFLOAT_S8_UINT";
        default:
            return fmt::format("Unknown ({})", vkFormat);
    }
}

String VkColorSpaceToString(VkColorSpaceKHR vkColorSpace)
{
    switch (vkColorSpace) {
        case VK_COLOR_SPACE_SRGB_NONLINEAR_KHR:
            return "VK_COLOR_SPACE_SRGB_NONLINEAR_KHR";
        case VK_COLOR_SPACE_DISPLAY_P3_NONLINEAR_EXT:
            return "VK_COLOR_SPACE_DISPLAY_P3_NONLINEAR_EXT";
        case VK_COLOR_SPACE_EXTENDED_SRGB_LINEAR_EXT:
            return "VK_COLOR_SPACE_EXTENDED_SRGB_LINEAR_EXT";
        case VK_COLOR_SPACE_DISPLAY_P3_LINEAR_EXT:
            return "VK_COLOR_SPACE_DISPLAY_P3_LINEAR_EXT";
        case VK_COLOR_SPACE_DCI_P3_NONLINEAR_EXT:
            return "VK_COLOR_SPACE_DCI_P3_NONLINEAR_EXT";
        case VK_COLOR_SPACE_BT709_LINEAR_EXT:
            return "VK_COLOR_SPACE_BT709_LINEAR_EXT";
        case VK_COLOR_SPACE_BT709_NONLINEAR_EXT:
            return "VK_COLOR_SPACE_BT709_NONLINEAR_EXT";
        case VK_COLOR_SPACE_BT2020_LINEAR_EXT:
            return "VK_COLOR_SPACE_BT2020_LINEAR_EXT";
        case VK_COLOR_SPACE_HDR10_ST2084_EXT:
            return "VK_COLOR_SPACE_HDR10_ST2084_EXT";
        case VK_COLOR_SPACE_DOLBYVISION_EXT:
            return "VK_COLOR_SPACE_DOLBYVISION_EXT";
        case VK_COLOR_SPACE_HDR10_HLG_EXT:
            return "VK_COLOR_SPACE_HDR10_HLG_EXT";
        case VK_COLOR_SPACE_ADOBERGB_LINEAR_EXT:
            return "VK_COLOR_SPACE_ADOBERGB_LINEAR_EXT";
        case VK_COLOR_SPACE_ADOBERGB_NONLINEAR_EXT:
            return "VK_COLOR_SPACE_ADOBERGB_NONLINEAR_EXT";
        case VK_COLOR_SPACE_PASS_THROUGH_EXT:
            return "VK_COLOR_SPACE_PASS_THROUGH_EXT";
        case VK_COLOR_SPACE_EXTENDED_SRGB_NONLINEAR_EXT:
            return "VK_COLOR_SPACE_EXTENDED_SRGB_NONLINEAR_EXT";
        default:
            return fmt::format("Unknown ({})", vkColorSpace);
    }
}

String VkPresentModeToString(VkPresentModeKHR vkPresentMode)
{
    switch (vkPresentMode) {
        case VK_PRESENT_MODE_IMMEDIATE_KHR:
            return "VK_PRESENT_MODE_IMMEDIATE_KHR";
        case VK_PRESENT_MODE_MAILBOX_KHR:
            return "VK_PRESENT_MODE_MAILBOX_KHR";
        case VK_PRESENT_MODE_FIFO_KHR:
            return "VK_PRESENT_MODE_FIFO_KHR";
        case VK_PRESENT_MODE_FIFO_RELAXED_KHR:
            return "VK_PRESENT_MODE_FIFO_RELAXED_KHR";
        default:
            return fmt::format("Unknown ({})", vkPresentMode);
    }
}

} // namespace Graphics

} // namespace ryme