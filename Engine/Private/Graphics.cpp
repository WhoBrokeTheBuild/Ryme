#include <Ryme/Graphics.hpp>
#include <Ryme/Ryme.hpp>
#include <Ryme/Buffer.hpp>
#include <Ryme/Shader.hpp>

#include <Ryme/ShaderGlobals.hpp>
#include <Ryme/ShaderTransform.hpp>

#include <SDL_vulkan.h>

RYME_DISABLE_WARNINGS()

    #define VMA_IMPLEMENTATION
    #include <vma/vk_mem_alloc.h>

RYME_ENABLE_WARNINGS()

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

namespace ryme {

namespace Graphics {

// Window

SDL_Window * _window = nullptr;

Vec2i _windowSize;

String _windowTitle;

// Vulkan Instance

List<vk::LayerProperties> _availableLayerList;

List<vk::ExtensionProperties> _availableInstanceExtensionList;

vk::Instance Instance;

vk::DebugUtilsMessengerEXT _debugUtilsMessenger;

// Vulkan Surface

vk::SurfaceKHR _surface;

// Vulkan Physical Device

vk::PhysicalDeviceProperties _physicalDeviceProperties;

vk::PhysicalDeviceFeatures _physicalDeviceFeatures;

vk::PhysicalDevice _physicalDevice;

// Vulkan Queues

uint32_t _graphicsQueueFamilyIndex;

uint32_t _presentQueueFamilyIndex;

vk::Queue _graphicsQueue;

vk::Queue _presentQueue;

// Vulkan Logical Device

List<vk::ExtensionProperties> _availableDeviceExtensionList;

vk::Device Device;

// Vulkan Memory Allocator

VmaAllocator Allocator;

// Vulkan Command Buffer

vk::CommandPool _commandPool;

List<vk::CommandBuffer> _commandBufferList;

// Swap Chain

vk::Extent2D _swapChainExtent;

vk::SwapchainKHR _swapChain;

vk::Format _swapChainImageFormat;

List<vk::Image> _swapChainImageList;

List<vk::ImageView> _swapChainImageViewList;

// Depth Buffer

vk::Format _depthImageFormat;

vk::Image _depthImage;

VmaAllocation _depthImageAllocation;

vk::ImageView _depthImageView;

// Render Pass

vk::RenderPass RenderPass;

// Descriptor Pool

vk::DescriptorPool _descriptorPool;

// Sync Objects

List<VkSemaphore> _imageAvailableSemaphoreList;

List<VkSemaphore> _renderingFinishedSemaphoreList;

List<VkFence> _inFlightFenceList;

List<VkFence> _imageInFlightList;

inline bool hasInstanceLayer(StringView name)
{
    for (const auto& layer : _availableLayerList) {
        if (layer.layerName == name) {
            return true;
        }
    }
    
    return false;
}

inline bool hasInstanceExtension(StringView name)
{
    for (const auto& extension : _availableInstanceExtensionList) {
        if (extension.extensionName == name) {
            return true;
        }
    }

    return false;
}

inline bool hasDeviceExtension(StringView name)
{
    for (const auto& extension : _availableDeviceExtensionList) {
        if (extension.extensionName == name) {
            return true;
        }
    }

    return false;
}

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
void ScriptInit(py::module m)
{
    // m.def_submodule("Graphics")
    //     .def("GetWindowSize", []() {
    //         int width, height;
    //         SDL_GetWindowSize(_sdlWindow, &width, &height);
    //         return std::make_tuple(width, height);
    //     })
    //     .def("SetWindowSize", [](int width, int height) {
    //         SDL_SetWindowSize(_sdlWindow, width, height);
    //     });
}

void initWindow()
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
    
    _window = SDL_CreateWindow(
        _windowTitle.c_str(),
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        _windowSize.x,
        _windowSize.y,
        SDL_WINDOW_VULKAN
    );

    if (!_window) {
        throw Exception("SDL_CreateWindow failed, {}", SDL_GetError());
    }

    VULKAN_HPP_DEFAULT_DISPATCHER.init(
        (PFN_vkGetInstanceProcAddr)SDL_Vulkan_GetVkGetInstanceProcAddr()
    );
}

void initInstance()
{
    vk::Result vkResult;

    // Layers

    _availableLayerList = vk::enumerateInstanceLayerProperties();

    List<const char *> requiredLayerNameList = { };
    
    if (hasInstanceLayer("VK_LAYER_KHRONOS_validation")) {
        requiredLayerNameList.push_back("VK_LAYER_KHRONOS_validation");
    }
    
    Log(RYME_ANCHOR, "Available Vulkan Layers:");
    for (const auto& layer : _availableLayerList) {
        Log(RYME_ANCHOR, "\t{}: {}", layer.layerName, layer.description);
    }

    Log(RYME_ANCHOR, "Required Vulkan Layers:");
    for (const auto& layer : requiredLayerNameList) {
        Log(RYME_ANCHOR, "\t{}", layer);
    }

    // Extensions

    _availableInstanceExtensionList = vk::enumerateInstanceExtensionProperties();

    uint32_t requiredInstanceExtensionCount = 0;
    SDL_Vulkan_GetInstanceExtensions(_window, &requiredInstanceExtensionCount, nullptr);

    List<const char *> requiredInstanceExtensionList(requiredInstanceExtensionCount);
    SDL_bool result = SDL_Vulkan_GetInstanceExtensions(
        _window,
        &requiredInstanceExtensionCount,
        requiredInstanceExtensionList.data()
    );

    if (!result) {
        throw Exception("SDL_Vulkan_GetInstanceExtensions failed, {}", SDL_GetError());
    }

    #if defined(VK_EXT_debug_utils)

        if (hasInstanceExtension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME)) {
            requiredInstanceExtensionList.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

    #endif

    Log(RYME_ANCHOR, "Available Vulkan Instance Extensions:");
    for (const auto& extension : _availableInstanceExtensionList) {
        Log(RYME_ANCHOR, "\t{}", extension.extensionName);
    }

    Log(RYME_ANCHOR, "Required Vulkan Instance Extensions:");
    for (const auto& extension : requiredInstanceExtensionList) {
        Log(RYME_ANCHOR, "\t{}", extension);
    }

    // Application Info

    const auto& applicationName = GetApplicationName();
    const auto& applicationVersion = GetApplicationVersion();
    const auto& engineVersion = GetVersion();

    auto applicationInfo = vk::ApplicationInfo()
        .setPApplicationName(applicationName.c_str())
        .setApplicationVersion(applicationVersion.ToVkVersion())
        .setPEngineName(RYME_PROJECT_NAME)
        .setEngineVersion(engineVersion.ToVkVersion())
        .setApiVersion(VK_API_VERSION_1_1);

    // Instance
    
    auto instanceCreateInfo = vk::InstanceCreateInfo()
        .setPApplicationInfo(&applicationInfo)
        .setPEnabledLayerNames(requiredLayerNameList)
        .setPEnabledExtensionNames(requiredInstanceExtensionList);

    #if defined(VK_EXT_debug_utils)

        auto debugUtilsMessengerCreateInfo = vk::DebugUtilsMessengerCreateInfoEXT()
            .setMessageSeverity( 
                vk::DebugUtilsMessageSeverityFlagBitsEXT::eError |
                vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
                vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo
            )
            .setMessageType(
                vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
                vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
                vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance
            )
            .setPfnUserCallback(_VulkanDebugMessageCallback);
            
        if (hasInstanceExtension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME)) {
            instanceCreateInfo.pNext = &debugUtilsMessengerCreateInfo;
        }

    #endif

    Instance = vk::createInstance(instanceCreateInfo);

    VULKAN_HPP_DEFAULT_DISPATCHER.init(Instance);

    Log(RYME_ANCHOR, "Vulkan Header Version: {}.{}.{}",
        VK_VERSION_MAJOR(VK_HEADER_VERSION_COMPLETE),
        VK_VERSION_MINOR(VK_HEADER_VERSION_COMPLETE),
        VK_VERSION_PATCH(VK_HEADER_VERSION_COMPLETE)
    );

    // Debug Utils Messenger

    #if defined(VK_EXT_debug_utils)

        if (hasInstanceExtension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME)) {
            _debugUtilsMessenger = Instance.createDebugUtilsMessengerEXT(debugUtilsMessengerCreateInfo);
        }

    #endif

}

void initSurface()
{
    SDL_bool result = SDL_Vulkan_CreateSurface(
        _window,
        Instance,
        reinterpret_cast<VkSurfaceKHR *>(&_surface)
    );
    
    if (!result) {
        throw Exception("SDL_Vulkan_CreateSurface() failed, {}", SDL_GetError());
    }
}

void initDevice()
{
    vk::Result vkResult;

    // Physical Device

    // TODO: Allow user to choose GPU

    Log(RYME_ANCHOR, "Available Physical Devices:");

    for (const auto& physicalDevice : Instance.enumeratePhysicalDevices()) {
        _physicalDeviceProperties = physicalDevice.getProperties();
        _physicalDeviceFeatures = physicalDevice.getFeatures();

        Log(RYME_ANCHOR, "\t{}", _physicalDeviceProperties.deviceName);
        
        bool isSuitable = (
            _physicalDeviceProperties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu &&
            _physicalDeviceFeatures.geometryShader
        );

        if (isSuitable) {
            _physicalDevice = physicalDevice;
            break;
        }
    }

    if (!_physicalDevice) {
        throw Exception("No suitable physical device found");
    }

    Log(RYME_ANCHOR, "Physical Device Name: {}", _physicalDeviceProperties.deviceName);

    Log(RYME_ANCHOR, "Physical Vulkan Version: {}.{}.{}",
        VK_VERSION_MAJOR(_physicalDeviceProperties.apiVersion),
        VK_VERSION_MINOR(_physicalDeviceProperties.apiVersion),
        VK_VERSION_PATCH(_physicalDeviceProperties.apiVersion)
    );

    // Queues

    _graphicsQueueFamilyIndex = UINT32_MAX;
    _presentQueueFamilyIndex = UINT32_MAX;

    auto queueFamilyPropertyList = _physicalDevice.getQueueFamilyProperties();

    Log(RYME_ANCHOR, "Available Vulkan Queue Families:");

    uint32_t index = 0;
    for (const auto& properties : queueFamilyPropertyList) {
        auto hasPresent = _physicalDevice.getSurfaceSupportKHR(index, _surface);
        bool hasGraphics = (properties.queueFlags & vk::QueueFlagBits::eGraphics ? true : false);

        // Pick the first available Queue with Graphics support
        if (_graphicsQueueFamilyIndex == UINT32_MAX && hasGraphics) {
            _graphicsQueueFamilyIndex = index;
        }
        
        // Pick the first available Queue with Present support
        if (_presentQueueFamilyIndex == UINT32_MAX && hasPresent) {
            _presentQueueFamilyIndex = index;
        }

        // If our Graphics and Present Queues aren't the same, try to find one that supports both
        if (_graphicsQueueFamilyIndex != _presentQueueFamilyIndex
            && hasPresent && hasGraphics) {
            _graphicsQueueFamilyIndex = index;
            _presentQueueFamilyIndex = index;
        }

        Log(RYME_ANCHOR, "\t#{}: {} Queues, {}",
            index,
            properties.queueCount,
            vk::to_string(properties.queueFlags)
        );

        ++index;
    }

    if (_graphicsQueueFamilyIndex == UINT32_MAX) {
        throw Exception("No suitable graphics queue found");
    }

    if (_presentQueueFamilyIndex == UINT32_MAX) {
        throw Exception("No suitable present queue found");
    }

    Log(RYME_ANCHOR, "Vulkan Graphics Queue Family Index: #{}", _graphicsQueueFamilyIndex);

    Log(RYME_ANCHOR, "Vulkan Present Queue Family Index: #{}", _presentQueueFamilyIndex);

    const float queuePriorities = 1.0f;

    Set<uint32_t> queueFamilyIndexSet = {
        _graphicsQueueFamilyIndex,
        _presentQueueFamilyIndex
    };

    List<vk::DeviceQueueCreateInfo> queueCreateInfoList;
    for (auto index : queueFamilyIndexSet) {
        queueCreateInfoList.push_back(
            vk::DeviceQueueCreateInfo()
                .setQueueFamilyIndex(index)
                .setQueueCount(1)
                .setPQueuePriorities(&queuePriorities)
        );
    }

    // Extensions

    _availableDeviceExtensionList = _physicalDevice.enumerateDeviceExtensionProperties();

    List<const char *> requiredDeviceExtensionNameList = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };
    
    #if defined(VK_EXT_memory_budget)
        
        if (hasDeviceExtension(VK_EXT_MEMORY_BUDGET_EXTENSION_NAME)) {
            requiredDeviceExtensionNameList.push_back(VK_EXT_MEMORY_BUDGET_EXTENSION_NAME);
        }

    #endif
    
    Log(RYME_ANCHOR, "Available Vulkan Device Extensions:");
    for (const auto& extension : _availableDeviceExtensionList) {
        Log(RYME_ANCHOR, "\t{}", extension.extensionName);
    }

    Log(RYME_ANCHOR, "Required Vulkan Device Extensions:");
    for (auto extension : requiredDeviceExtensionNameList) {
        Log(RYME_ANCHOR, "\t{}", extension);
    }

    // Device

    auto deviceCreateInfo = vk::DeviceCreateInfo()
        .setQueueCreateInfos(queueCreateInfoList)
        .setPEnabledExtensionNames(requiredDeviceExtensionNameList);

    Device = _physicalDevice.createDevice(deviceCreateInfo);

    VULKAN_HPP_DEFAULT_DISPATCHER.init(Device);
    
    _graphicsQueue = Device.getQueue(_graphicsQueueFamilyIndex, 0);
    _presentQueue = Device.getQueue(_presentQueueFamilyIndex, 0);

}

void initAllocator()
{
    VmaAllocatorCreateFlags allocatorCreateFlags = 0;

    #if defined(VK_EXT_memory_budget)
        
        if (hasDeviceExtension(VK_EXT_MEMORY_BUDGET_EXTENSION_NAME)) {
            allocatorCreateFlags |= VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT;
        }

    #endif

    auto allocatorCreateInfo = VmaAllocatorCreateInfo{
        .flags = allocatorCreateFlags,
        .physicalDevice = _physicalDevice,
        .device = Device,
        .instance = Instance,
        .vulkanApiVersion = VK_API_VERSION_1_1,
    };

    auto vkResult = (vk::Result)vmaCreateAllocator(&allocatorCreateInfo, &Allocator);

    vk::resultCheck(vkResult, "vmaCreateAllocator");

    auto memoryProperties = _physicalDevice.getMemoryProperties();

    List<VmaBudget> budgetList(memoryProperties.memoryHeapCount);
    vmaGetHeapBudgets(Allocator, budgetList.data());

    for (uint32_t heap = 0; heap < memoryProperties.memoryHeapCount; ++heap) {
        Log(RYME_ANCHOR, "Vulkan Memory Heap #{}: {}",
            heap,
            FormatBytesHumanReadable(budgetList[heap].budget)
        );

        for (uint32_t type = 0; type < memoryProperties.memoryTypeCount; ++type) {
            if (memoryProperties.memoryTypes[type].heapIndex == heap) {
                if (memoryProperties.memoryTypes[type].propertyFlags) {
                    Log(RYME_ANCHOR, "\tType #{}: {}",
                        type,
                        vk::to_string(memoryProperties.memoryTypes[type].propertyFlags)
                    );
                }
            }
        }
    }
}

void initDepthBuffer()
{
    List<vk::Format> potentialFormatList = {
        vk::Format::eD32Sfloat,
        vk::Format::eD32SfloatS8Uint,
        vk::Format::eD24UnormS8Uint,
        vk::Format::eD16Unorm,
        vk::Format::eD16UnormS8Uint,
    };

    _depthImageFormat = vk::Format::eUndefined;

    for (const auto& format : potentialFormatList) {
        vk::FormatProperties formatProperties = _physicalDevice.getFormatProperties(format);

        if (formatProperties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eDepthStencilAttachment) {
            _depthImageFormat = format;
            break;
        }
    }

    if (_depthImageFormat == vk::Format::eUndefined) {
        throw Exception("Unable to find suitable depth buffer image format");
    }

    Log(RYME_ANCHOR, "Vulkan Depth Buffer Image Format: {}",
        vk::to_string(_depthImageFormat)
    );
    
    vmaFreeMemory(Allocator, _depthImageAllocation);

    Device.destroyImage(_depthImage);

    auto imageCreateInfo = vk::ImageCreateInfo()
        .setImageType(vk::ImageType::e2D)
        .setFormat(_depthImageFormat)
        .setExtent(vk::Extent3D(_swapChainExtent, 1))
        .setMipLevels(1)
        .setArrayLayers(1)
        .setTiling(vk::ImageTiling::eOptimal)
        .setUsage(vk::ImageUsageFlagBits::eDepthStencilAttachment);

    auto allocationCreateInfo = VmaAllocationCreateInfo{
        .usage = VMA_MEMORY_USAGE_GPU_ONLY,
    };
    
    std::tie(_depthImage, _depthImageAllocation) = CreateImage(
        imageCreateInfo,
        allocationCreateInfo
    );

    Device.destroyImageView(_depthImageView);

    auto imageViewCreateInfo = vk::ImageViewCreateInfo()
        .setImage(_depthImage)
        .setViewType(vk::ImageViewType::e2D)
        .setFormat(_depthImageFormat)
        .setSubresourceRange({ vk::ImageAspectFlagBits::eDepth, 0, 1, 0, 1 });

    _depthImageView = Device.createImageView(imageViewCreateInfo);
}

void initRenderPass()
{
    auto colorAttachmentDescription = vk::AttachmentDescription()
        .setFormat(_swapChainImageFormat)
        .setSamples(vk::SampleCountFlagBits::e1)
        .setLoadOp(vk::AttachmentLoadOp::eClear)
        .setStoreOp(vk::AttachmentStoreOp::eStore)
        .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
        .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
        .setInitialLayout(vk::ImageLayout::eUndefined)
        .setFinalLayout(vk::ImageLayout::ePresentSrcKHR);

    auto colorAttachmentReference = vk::AttachmentReference()
        .setAttachment(0)
        .setLayout(vk::ImageLayout::eColorAttachmentOptimal);

    auto depthAttachmentDescription = vk::AttachmentDescription()
        .setFormat(_depthImageFormat)
        .setSamples(vk::SampleCountFlagBits::e1)
        .setLoadOp(vk::AttachmentLoadOp::eClear)
        .setStoreOp(vk::AttachmentStoreOp::eDontCare)
        .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
        .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
        .setInitialLayout(vk::ImageLayout::eUndefined)
        .setFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

    auto depthAttachmentReference = vk::AttachmentReference()
        .setAttachment(1)
        .setLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

    auto subpassDescription = vk::SubpassDescription()
        .setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
        .setColorAttachmentCount(1)
        .setPColorAttachments(&colorAttachmentReference)
        .setPDepthStencilAttachment(&depthAttachmentReference);

    auto subpassDependency = vk::SubpassDependency()
        .setSrcSubpass(VK_SUBPASS_EXTERNAL)
        .setDstSubpass(0)
        .setSrcStageMask(
            vk::PipelineStageFlagBits::eColorAttachmentOutput |
            vk::PipelineStageFlagBits::eEarlyFragmentTests
        )
        .setDstStageMask(
            vk::PipelineStageFlagBits::eColorAttachmentOutput |
            vk::PipelineStageFlagBits::eEarlyFragmentTests
        )
        .setSrcAccessMask({})
        .setDstAccessMask(
            vk::AccessFlagBits::eColorAttachmentWrite |
            vk::AccessFlagBits::eDepthStencilAttachmentWrite
        );

    Array<vk::AttachmentDescription, 2> attachmentList = {
        colorAttachmentDescription,
        depthAttachmentDescription,
    };

    Device.destroyRenderPass(RenderPass);

    auto renderPassCreateInfo = vk::RenderPassCreateInfo()
        .setAttachments(attachmentList)
        .setSubpassCount(1)
        .setPSubpasses(&subpassDescription)
        .setDependencyCount(1)
        .setPDependencies(&subpassDependency);

    RenderPass = Device.createRenderPass(renderPassCreateInfo);
}

void initUniformBuffers()
{
    Buffer shaderGlobalsBuffer;
    shaderGlobalsBuffer.Create(
        sizeof(ShaderGlobals),
        nullptr,
        vk::BufferUsageFlagBits::eUniformBuffer,
        VMA_MEMORY_USAGE_CPU_TO_GPU
    );

}

void initCommandBufferList()
{
    if (!_commandBufferList.empty()) {
        Device.freeCommandBuffers(_commandPool, _commandBufferList);
    }

    Device.destroyCommandPool(_commandPool);

    _commandPool = Device.createCommandPool(
        vk::CommandPoolCreateInfo({}, _graphicsQueueFamilyIndex)
    );

    _commandBufferList = Device.allocateCommandBuffers(
        vk::CommandBufferAllocateInfo(
            _commandPool,
            vk::CommandBufferLevel::ePrimary,
            _swapChainImageList.size()
        )
    );
}

void initSwapChain()
{
    RYME_BENCHMARK_START();

    vkDeviceWaitIdle(Device);

    /// Image Format

    auto formatList = _physicalDevice.getSurfaceFormatsKHR(_surface);

    if (formatList.empty()) {
        throw Exception("No Vulkan Surface Formats Available");
    }

    auto imageFormat = formatList.front();

    Log(RYME_ANCHOR, "Available Vulkan Surface Formats:");
    for (const auto& format : formatList) {
        bool isSRGB = (
            format.format == vk::Format::eR8G8B8A8Srgb ||
            format.format == vk::Format::eB8G8R8A8Srgb
        );

        if (isSRGB && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
            imageFormat = format;
        }

        Log(RYME_ANCHOR, "\t{} {}",
            vk::to_string(format.format),
            vk::to_string(format.colorSpace)
        );
    }

    _swapChainImageFormat = imageFormat.format;

    Log(RYME_ANCHOR, "Vulkan Swap Chain Image Format: {} {} ",
        vk::to_string(imageFormat.format),
        vk::to_string(imageFormat.colorSpace)
    );

    /// Image Extent

    auto surfaceCapabilities = _physicalDevice.getSurfaceCapabilitiesKHR(_surface);

    _swapChainExtent = surfaceCapabilities.currentExtent;

    if (_swapChainExtent.width == UINT32_MAX) {
        _swapChainExtent.width = std::clamp(
            static_cast<uint32_t>(_windowSize.x),
            surfaceCapabilities.minImageExtent.width,
            surfaceCapabilities.maxImageExtent.width
        );

        _swapChainExtent.height = std::clamp(
            static_cast<uint32_t>(_windowSize.y),
            surfaceCapabilities.minImageExtent.height,
            surfaceCapabilities.maxImageExtent.height
        );
    }

    Log(RYME_ANCHOR, "Vulkan Swap Chain Extent: {}x{}",
        _swapChainExtent.width,
        _swapChainExtent.height
    );

    /// Present Mode

    auto presentModeList = _physicalDevice.getSurfacePresentModesKHR(_surface);

    // FIFO is the only present mode required to be supported
    auto presentMode = vk::PresentModeKHR::eFifo;
    
    Log(RYME_ANCHOR, "Available Vulkan Present Modes:");
    for (const auto& mode : presentModeList) {
        Log(RYME_ANCHOR, "\t{}", vk::to_string(mode));
    }
    
    // Mailbox is like FIFO, but discards extra images
    if (ListContains(presentModeList, vk::PresentModeKHR::eMailbox)) {
        presentMode = vk::PresentModeKHR::eMailbox;
    }

    Log(RYME_ANCHOR, "Vulkan Swap Chain Present Mode: {}", vk::to_string(presentMode));

    /// Swap Chain

    auto oldSwapChain = _swapChain;

    auto swapChainCreateInfo = vk::SwapchainCreateInfoKHR()
        .setSurface(_surface)
        .setMinImageCount(surfaceCapabilities.minImageCount)
        .setImageFormat(imageFormat.format)
        .setImageColorSpace(imageFormat.colorSpace)
        .setImageExtent(_swapChainExtent)
        .setImageArrayLayers(1) // Always 1 for 3D applications
        .setImageUsage(vk::ImageUsageFlagBits::eColorAttachment)
        .setPreTransform(surfaceCapabilities.currentTransform)
        .setPresentMode(presentMode)
        .setClipped(true)
        .setOldSwapchain(oldSwapChain);
        
    if (surfaceCapabilities.supportedCompositeAlpha & vk::CompositeAlphaFlagBitsKHR::ePreMultiplied) {
        swapChainCreateInfo.setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::ePreMultiplied);
    }
    else if (surfaceCapabilities.supportedCompositeAlpha & vk::CompositeAlphaFlagBitsKHR::ePostMultiplied) {
        swapChainCreateInfo.setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::ePostMultiplied);
    }
    else if (surfaceCapabilities.supportedCompositeAlpha & vk::CompositeAlphaFlagBitsKHR::eInherit) {
        swapChainCreateInfo.setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eInherit);
    }
    
    Array<uint32_t, 2> queueFamilyIndexList = {
        _graphicsQueueFamilyIndex,
        _presentQueueFamilyIndex
    };

    if (_graphicsQueueFamilyIndex != _presentQueueFamilyIndex) {
        swapChainCreateInfo.setImageSharingMode(vk::SharingMode::eConcurrent);
        swapChainCreateInfo.setQueueFamilyIndices(queueFamilyIndexList);
    }

    Log(RYME_ANCHOR, "Vulkan Swap Chain Image Count: {}",
        swapChainCreateInfo.minImageCount
    );

    Log(RYME_ANCHOR, "Vulkan Swap Chain Pre-Transform: {}",
        vk::to_string(swapChainCreateInfo.preTransform)
    );

    Log(RYME_ANCHOR, "Vulkan Swap Chain Composite Alpha: {}",
        vk::to_string(swapChainCreateInfo.compositeAlpha)
    );

    Log(RYME_ANCHOR, "Vulkan Swap Chain Sharing Mode: {}",
        vk::to_string(swapChainCreateInfo.imageSharingMode)
    );

    _swapChain = Device.createSwapchainKHR(swapChainCreateInfo);

    Device.destroySwapchainKHR(oldSwapChain);

    /// Image List

    _swapChainImageList = Device.getSwapchainImagesKHR(_swapChain);

    for (auto& imageView : _swapChainImageViewList) {
        Device.destroyImageView(imageView);
    }

    _swapChainImageViewList.clear();
    _swapChainImageViewList.reserve(_swapChainImageList.size());

    auto imageViewCreateInfo = vk::ImageViewCreateInfo()
        .setViewType(vk::ImageViewType::e2D)
        .setFormat(_swapChainImageFormat)
        .setSubresourceRange({ vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 });

    for (const auto& image : _swapChainImageList) {
        imageViewCreateInfo.setImage(image);
        _swapChainImageViewList.push_back(
            Device.createImageView(imageViewCreateInfo)
        );
    }

    initDepthBuffer();
    initRenderPass();
    // initFramebufferList();
    initCommandBufferList();

    RYME_BENCHMARK_END();
}

RYME_API
void Init(String windowTitle, Vec2i windowSize)
{
    RYME_BENCHMARK_START();

    _windowSize = windowSize;
    _windowTitle = windowTitle;
    
    initWindow();
    initInstance();
    initSurface();
    initDevice();
    initAllocator();

    initSwapChain();
    initSwapChain(); // Test swap chain recreation
    
    RYME_BENCHMARK_END();
}

RYME_API
void Term()
{
    RYME_BENCHMARK_START();

    Device.freeCommandBuffers(_commandPool, _commandBufferList);

    Device.destroyCommandPool(_commandPool);

    Device.destroyRenderPass(RenderPass);

    Device.destroyImageView(_depthImageView);
    
    Device.destroyImage(_depthImage);

    vmaFreeMemory(Allocator, _depthImageAllocation);

    for (auto& imageView : _swapChainImageViewList) {
        Device.destroyImageView(imageView);
    }

    // The vk::Image's in _swapChainImageList are destroyed as well
    Device.destroySwapchainKHR(_swapChain);

    vmaDestroyAllocator(Allocator);

    Device.destroy();

    Instance.destroySurfaceKHR(_surface);

    Instance.destroyDebugUtilsMessengerEXT(_debugUtilsMessenger);

    Instance.destroy();

    if (_window) {
        SDL_DestroyWindow(_window);
        _window = nullptr;
    }

    SDL_Quit();

    RYME_BENCHMARK_END();
}

RYME_API
void SetWindowTitle(String windowTitle)
{
    assert(_window);
    
    _windowTitle = windowTitle;
    SDL_SetWindowTitle(_window, _windowTitle.c_str());
}

RYME_API
String GetWindowTitle()
{
    return _windowTitle;
}

RYME_API
void SetWindowSize(Vec2i windowSize)
{
    assert(_window);
    
    _windowSize = windowSize;
    SDL_SetWindowSize(_window, _windowSize.x, _windowSize.y);
}

RYME_API
Vec2i GetWindowSize()
{
    return _windowSize;
}

// void initSwapChain()
// {
//     VkResult vkResult;

//     uint32_t formatCount = 0;
//     vkGetPhysicalDeviceSurfaceFormatsKHR(
//         _vkPhysicalDevice,
//         _vkSurface,
//         &formatCount,
//         nullptr
//     );

//     if (formatCount == 0) {
//         throw Exception("vkGetPhysicalDeviceSurfaceFormatsKHR() failed, no surface formats found");
//     }

//     List<VkSurfaceFormatKHR> formatList(formatCount);
//     vkGetPhysicalDeviceSurfaceFormatsKHR(
//         _vkPhysicalDevice,
//         _vkSurface,
//         &formatCount,
//         formatList.data()
//     );

//     Log(RYME_ANCHOR, "Available Vulkan Surface Formats:");
//     VkSurfaceFormatKHR surfaceFormat = formatList[0];
//     for (const auto& format : formatList) {
//         bool isSRGB = (
//             format.format == VK_FORMAT_R8G8B8A8_SRGB ||
//             format.format == VK_FORMAT_B8G8R8A8_SRGB
//         );

//         if (isSRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
//             surfaceFormat = format;
//         }

//         Log(RYME_ANCHOR, "\t{} {}",
//             std::to_string(format.format),
//             std::to_string(format.colorSpace)
//         );
//     }

//     Log(RYME_ANCHOR, "Vulkan Swap Chain Image Format: {}",
//         std::to_string(surfaceFormat.format));

//     Log(RYME_ANCHOR, "Vuilkan Swap Chain Image Color Space: {}",
//         std::to_string(surfaceFormat.colorSpace));

//     uint32_t presentModeCount = 0;
//     vkGetPhysicalDeviceSurfacePresentModesKHR(
//         _vkPhysicalDevice,
//         _vkSurface,
//         &presentModeCount,
//         nullptr
//     );

//     if (presentModeCount == 0) {
//         throw Exception("vkGetPhysicalDeviceSurfacePresentModeKHR() failed, no present modes found");
//     }

//     List<VkPresentModeKHR> presentModeList(presentModeCount);
//     vkGetPhysicalDeviceSurfacePresentModesKHR(
//         _vkPhysicalDevice,
//         _vkSurface,
//         &presentModeCount,
//         presentModeList.data()
//     );

//     // VK_PRESENT_MODE_IMMEDIATE_KHR = Do not wait for vsync, may cause screen tearing
//     // VK_PRESENT_MODE_FIFO_KHR = Queue of presentation requests, wait for vsync, required to be supported
//     //    equivalent to {wgl|glX|egl}SwapBuffers(1)
//     // VK_PRESENT_MODE_FIFO_RELAXED_KHR = Similar to FIFO, but will not wait for a second vsync period 
//     //    if the first has already passed, may cause screen tearing
//     //    equivalent to {wgl|glX}SwapBuffers(-1)
//     // VK_PRESENT_MODE_MAILBOX_KHR = Queue of presentation requests, wait for vsync, replaces entries if the queue is full

//     Log(RYME_ANCHOR, "Available Vulkan Present Modes:");

//     // FIFO is the only present mode required to be supported
//     VkPresentModeKHR swapChainPresentMode = VK_PRESENT_MODE_FIFO_KHR;
//     for (const auto& presentMode : presentModeList) {
//         if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
//             swapChainPresentMode = presentMode;
//         }

//         Log(RYME_ANCHOR, "\t{}", std::to_string(presentMode));
//     }

//     Log(RYME_ANCHOR, "Vulkan Swap Chain Present Mode: {}",
//         std::to_string(swapChainPresentMode));

//     VkSurfaceCapabilitiesKHR surfaceCapabilities;
//     vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
//         _vkPhysicalDevice,
//         _vkSurface,
//         &surfaceCapabilities
//     );

//     _vkSwapChainExtent = surfaceCapabilities.currentExtent;

//     if (_vkSwapChainExtent.width == UINT32_MAX || _vkSwapChainExtent.height == UINT32_MAX) {
//         const Vec2i& size = _windowSize;

//         _vkSwapChainExtent.width = std::clamp(
//             static_cast<uint32_t>(size.x),
//             surfaceCapabilities.minImageExtent.width,
//             surfaceCapabilities.maxImageExtent.width
//         );

//         _vkSwapChainExtent.height = std::clamp(
//             static_cast<uint32_t>(size.y),
//             surfaceCapabilities.minImageExtent.height,
//             surfaceCapabilities.maxImageExtent.height
//         );
//     }

//     Log(RYME_ANCHOR, "Vulkan Swap Chain Extent: {}x{}",
//         _vkSwapChainExtent.width,
//         _vkSwapChainExtent.height
//     );

//     uint32_t imageCount = std::clamp(
//         (unsigned)_backbufferCount,
//         surfaceCapabilities.minImageCount,
//         surfaceCapabilities.maxImageCount
//     );

//     Log(RYME_ANCHOR, "Vulkan Swap Chain Image Count: {}", imageCount);

//     VkSwapchainKHR oldSwapChain = _vkSwapChain;

//     uint32_t queueFamilyIndices[] = {
//         _vkGraphicsQueueFamilyIndex,
//         _vkPresentQueueFamilyIndex,
//     };

//     VkSharingMode sharingMode = (
//         _vkGraphicsQueueFamilyIndex == _vkPresentQueueFamilyIndex
//         ? VK_SHARING_MODE_EXCLUSIVE
//         : VK_SHARING_MODE_CONCURRENT
//     );

//     VkSwapchainCreateInfoKHR swapChainCreateInfo = {
//         .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
//         .pNext = nullptr,
//         .flags = 0,
//         .surface = _vkSurface,
//         .minImageCount = imageCount,
//         .imageFormat = surfaceFormat.format,
//         .imageColorSpace = surfaceFormat.colorSpace,
//         .imageExtent = _vkSwapChainExtent,
//         .imageArrayLayers = 1,
//         .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
//         .imageSharingMode = sharingMode,
//         .queueFamilyIndexCount = 2,
//         .pQueueFamilyIndices = queueFamilyIndices,
//         .preTransform = surfaceCapabilities.currentTransform,
//         .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
//         .presentMode = swapChainPresentMode,
//         .clipped = VK_TRUE,
//         .oldSwapchain = oldSwapChain,
//     };

//     vkResult = vkCreateSwapchainKHR(
//         Device,
//         &swapChainCreateInfo,
//         nullptr,
//         &_vkSwapChain
//     );

//     if (vkResult != VK_SUCCESS) {
//         throw Exception("vkCreateSwapchainKHR() failed");
//     }

//     if (oldSwapChain) {
//         vkDestroySwapchainKHR(Device, oldSwapChain, nullptr);
//         oldSwapChain = VK_NULL_HANDLE;
//     }

//     _vkSwapChainImageFormat = surfaceFormat.format;

//     vkGetSwapchainImagesKHR(
//         Device,
//         _vkSwapChain,
//         &imageCount,
//         nullptr
//     );

//     _vkSwapChainImageList.resize(imageCount, VK_NULL_HANDLE);

//     vkGetSwapchainImagesKHR(
//         Device,
//         _vkSwapChain,
//         &imageCount,
//         _vkSwapChainImageList.data()
//     );

//     _vkSwapChainImageViewList.resize(imageCount, VK_NULL_HANDLE);

//     for (unsigned i = 0; i < _backbufferCount; ++i) {
//         if (_vkSwapChainImageViewList[i]) {
//             vkDestroyImageView(Device, _vkSwapChainImageViewList[i], nullptr);
//             _vkSwapChainImageViewList[i] = VK_NULL_HANDLE;
//         }

//         VkImageViewCreateInfo imageViewCreateInfo = {
//             .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
//             .pNext = nullptr,
//             .flags = 0,
//             .image = _vkSwapChainImageList[i],
//             .viewType = VK_IMAGE_VIEW_TYPE_2D,
//             .format = _vkSwapChainImageFormat,
//             .subresourceRange = {
//                 .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
//                 .baseMipLevel = 0,
//                 .levelCount = 1,
//                 .baseArrayLayer = 0,
//                 .layerCount = 1,
//             },
//         };

//         vkResult = vkCreateImageView(
//             Device,
//             &imageViewCreateInfo,
//             nullptr,
//             &_vkSwapChainImageViewList[i]
//         );
        
//         if (vkResult != VK_SUCCESS) {
//             throw Exception("vkCreateImageView() failed for image view #{}", i);
//         }

//         _backbufferCount = imageCount;

//         initDepthBuffer();
//         initRenderPass();
//         // initDescriptorPool();
//         // initPipelineLayout();
//         // initFramebuffers();
//         // initCommandBuffers();
//     }
// }

// void termSwapChain()
// {
//     // termCommandBuffers();
//     // termFramebuffers();
//     // termPipelineLayout();
//     // termDescriptorPool();
//     termRenderPass();
//     termDepthBuffer();

//     for (auto& imageView : _vkSwapChainImageViewList) {
//         if (imageView) {
//             vkDestroyImageView(Device, imageView, nullptr);
//             imageView = nullptr;
//         }
//     }

//     if (_vkSwapChain) {
//         vkDestroySwapchainKHR(Device, _vkSwapChain, nullptr);
//         _vkSwapChain = VK_NULL_HANDLE;
//     }
// }

// void resetSwapChain()
// {
//     if (_vkSwapChain) {
//         vkDeviceWaitIdle(Device);

//         initSwapChain();
//     }
// }

// void initSyncObjects()
// {
//     VkResult vkResult;

//     _vkImageAvailableSemaphoreList.resize(_backbufferCount, VK_NULL_HANDLE);
//     _vkRenderingFinishedSemaphoreList.resize(_backbufferCount, VK_NULL_HANDLE);
//     _vkInFlightFenceList.resize(_backbufferCount, VK_NULL_HANDLE);
//     _vkImageInFlightList.resize(_backbufferCount, VK_NULL_HANDLE);

//     VkSemaphoreCreateInfo semaphoreCreateInfo = {
//         .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
//         .pNext = nullptr,
//         .flags = 0,
//     };

//     VkFenceCreateInfo fenceCreateInfo = {
//         .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
//         .pNext = nullptr,
//         .flags = VK_FENCE_CREATE_SIGNALED_BIT,
//     };

//     for (unsigned i = 0; i < _backbufferCount; ++i) {
//         vkResult = vkCreateSemaphore(
//             Device,
//             &semaphoreCreateInfo,
//             nullptr,
//             &_vkImageAvailableSemaphoreList[i]
//         );

//         if (vkResult != VK_SUCCESS) {
//             throw Exception("vkCreateSemaphore() failed");
//         }   

//         vkResult = vkCreateSemaphore(
//             Device,
//             &semaphoreCreateInfo,
//             nullptr,
//             &_vkRenderingFinishedSemaphoreList[i]
//         );

//         if (vkResult != VK_SUCCESS) {
//             throw Exception("vkCreateSemaphore() failed");
//         }

//         vkResult = vkCreateFence(
//             Device,
//             &fenceCreateInfo,
//             nullptr,
//             &_vkInFlightFenceList[i]
//         );

//         if (vkResult != VK_SUCCESS) {
//             throw Exception("vkCreateFence() failed");
//         }
//     }
// }

// void termSyncObjects()
// {
//     for (auto& fence : _vkImageInFlightList) {
//         // TODO: This could blow up be careful.
//         vkDestroyFence(Device, fence, VK_NULL_HANDLE);
//         fence = VK_NULL_HANDLE;
//     }

//     for (auto& fence : _vkInFlightFenceList) {
//         vkDestroyFence(Device, fence, VK_NULL_HANDLE);
//         fence = VK_NULL_HANDLE;
//     }

//     for (auto& semaphore : _vkRenderingFinishedSemaphoreList) {
//         vkDestroySemaphore(Device, semaphore, VK_NULL_HANDLE);
//         semaphore = VK_NULL_HANDLE;
//     }

//     for (auto& semaphore : _vkImageAvailableSemaphoreList) {
//         vkDestroySemaphore(Device, semaphore, VK_NULL_HANDLE);
//         semaphore = VK_NULL_HANDLE;
//     }
// }

// void initDepthBuffer()
// {
//     VkResult vkResult;

//     termDepthBuffer();

//     // TODO: Investigate
//     List<VkFormat> potentialFormatList = {
//         VK_FORMAT_D32_SFLOAT,
//         VK_FORMAT_D32_SFLOAT_S8_UINT,
//         VK_FORMAT_D24_UNORM_S8_UINT,
//         VK_FORMAT_D16_UNORM,
//         VK_FORMAT_D16_UNORM_S8_UINT,
//     };

//     _vkDepthImageFormat = VK_FORMAT_UNDEFINED;

//     for (auto format : potentialFormatList) {
//         VkFormatProperties formatProperties;
//         vkGetPhysicalDeviceFormatProperties(_vkPhysicalDevice, format, &formatProperties);

//         bool isSuitable = (
//             (formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) > 0
//         );

//         if (isSuitable) {
//             _vkDepthImageFormat = format;
//             break;
//         }
//     }

//     if (_vkDepthImageFormat == VK_FORMAT_UNDEFINED) {
//         throw Exception("Unable to find suitable depth buffer image format");
//     }

//     Log(RYME_ANCHOR, "Vulkan Depth Buffer Image Format: {}",
//         std::to_string(_vkDepthImageFormat));

//     VkImageCreateInfo imageCreateInfo = {
//         .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
//         .pNext = nullptr,
//         .flags = 0,
//         .imageType = VK_IMAGE_TYPE_2D,
//         .format = _vkDepthImageFormat,
//         .extent = {
//             .width = _vkSwapChainExtent.width,
//             .height = _vkSwapChainExtent.height,
//             .depth = 1,
//         },
//         .mipLevels = 1,
//         .arrayLayers = 1,
//         .samples = VK_SAMPLE_COUNT_1_BIT,
//         .tiling = VK_IMAGE_TILING_OPTIMAL,
//         .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
//         .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
//         .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
//     };

//     VmaAllocationCreateInfo allocationCreateInfo = {
//         .flags = 0,
//         .usage = VMA_MEMORY_USAGE_GPU_ONLY,
//     };

//     vkResult = vmaCreateImage(
//         Allocator,
//         &imageCreateInfo,
//         &allocationCreateInfo,
//         &_vkDepthImage,
//         &_vmaDepthImageAllocation,
//         nullptr
//     );

//     if (vkResult != VK_SUCCESS) {
//         throw Exception("vmaCreateImage() failed, unable to create depth buffer image");
//     }

//     VkImageViewCreateInfo imageViewCreateInfo = {
//         .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
//         .pNext = nullptr,
//         .flags = 0,
//         .image = _vkDepthImage,
//         .viewType = VK_IMAGE_VIEW_TYPE_2D,
//         .format = _vkDepthImageFormat,
//         .subresourceRange = {
//             .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
//             .baseMipLevel = 0,
//             .levelCount = 1,
//             .baseArrayLayer = 0,
//             .layerCount = 1,
//         },
//     };

//     vkResult = vkCreateImageView(
//         Device,
//         &imageViewCreateInfo,
//         nullptr,
//         &_vkDepthImageView
//     );

//     if (vkResult != VK_SUCCESS) {
//         throw Exception("vkCreateImageView failed, unable to create depth buffer image view");
//     }
// }

// void termDepthBuffer()
// {
//     if (_vkDepthImage) {
//         vkDestroyImage(Device, _vkDepthImage, nullptr);
//         _vkDepthImage = VK_NULL_HANDLE;
//     }

//     if (_vmaDepthImageAllocation) {
//         vmaFreeMemory(Allocator, _vmaDepthImageAllocation);
//         _vmaDepthImageAllocation = VK_NULL_HANDLE;
//     }

//     if (_vkDepthImageView) {
//         vkDestroyImageView(Device, _vkDepthImageView, nullptr);
//         _vkDepthImageView = VK_NULL_HANDLE;
//     }
// }

// void initRenderPass()
// {
//     VkResult vkResult;

//     termRenderPass();

//     VkAttachmentDescription colorAttachmentDescription = {
//         .flags = 0,
//         .format = _vkSwapChainImageFormat,
//         .samples = VK_SAMPLE_COUNT_1_BIT,
//         .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
//         .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
//         .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
//         .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
//         .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
//         .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
//     };

//     VkAttachmentDescription depthAttachmentDescription = {
//         .flags = 0,
//         .format = _vkDepthImageFormat,
//         .samples = VK_SAMPLE_COUNT_1_BIT,
//         .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
//         .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
//         .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
//         .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
//         .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
//         .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
//     };

//     Array<VkAttachmentDescription, 2> attachmentList = {
//         colorAttachmentDescription,
//         depthAttachmentDescription,
//     };

//     VkAttachmentReference colorAttachmentReference = {
//         .attachment = 0,
//         .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
//     };

//     VkAttachmentReference depthAttachmentReference = {
//         .attachment = 1,
//         .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
//     };

//     Array<VkSubpassDescription, 1> subpassDescriptionList = {
//         VkSubpassDescription {
//             .flags = 0,
//             .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
//             .inputAttachmentCount = 0,
//             .pInputAttachments = nullptr,
//             .colorAttachmentCount = 1,
//             .pColorAttachments = &colorAttachmentReference,
//             .pResolveAttachments = nullptr,
//             .pDepthStencilAttachment = &depthAttachmentReference,
//             .preserveAttachmentCount = 0,
//             .pPreserveAttachments = nullptr,
//         },
//     };

//     Array<VkSubpassDependency, 1> subpassDependencyList = {
//         VkSubpassDependency {
//             .srcSubpass = VK_SUBPASS_EXTERNAL,
//             .dstSubpass = 0,
//             .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
//             .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
//             .srcAccessMask = 0,
//             .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
//         },
//     };

//     VkRenderPassCreateInfo renderPassCreateInfo = {
//         .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
//         .pNext = nullptr,
//         .flags = 0,
//         .attachmentCount = static_cast<uint32_t>(attachmentList.size()),
//         .pAttachments = attachmentList.data(),
//         .subpassCount = static_cast<uint32_t>(subpassDescriptionList.size()),
//         .pSubpasses = subpassDescriptionList.data(),
//         .dependencyCount = static_cast<uint32_t>(subpassDependencyList.size()),
//         .pDependencies = subpassDependencyList.data(),
//     };

//     vkResult = vkCreateRenderPass(Device, &renderPassCreateInfo, nullptr, &_vkRenderPass);

//     if (vkResult != VK_SUCCESS) {
//         throw Exception("vkCreateRenderPass() failed");
//     }
// }

// void termRenderPass()
// {
//     if (_vkRenderPass) {
//         vkDestroyRenderPass(Device, _vkRenderPass, nullptr);
//         _vkRenderPass = VK_NULL_HANDLE;
//     }
// }

// void initDescriptorPool()
// {
//     VkResult vkResult;

//     termDescriptorPool();

//     Array<VkDescriptorPoolSize, 1> descriptorPoolSizeList = {
//         VkDescriptorPoolSize {
//             .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
//             .descriptorCount = static_cast<uint32_t>(1), // materialList.size() + meshList.size(), can never be 0
//         },
//     };

//     VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {
//         .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
//         .pNext = nullptr,
//         .flags = 0,
//         .maxSets = _backbufferCount,
//         .poolSizeCount = static_cast<uint32_t>(descriptorPoolSizeList.size()),
//         .pPoolSizes = descriptorPoolSizeList.data(),
//     };

//     vkResult = vkCreateDescriptorPool(
//         Device,
//         &descriptorPoolCreateInfo,
//         nullptr,
//         &_vkDescriptorPool
//     );

//     if (vkResult != VK_SUCCESS) {
//         throw Exception("vkCreateDescriptorPool() failed");
//     }

//     Array<VkDescriptorSetLayoutBinding, 2> descriptorSetLayoutBindingList = {
//         VkDescriptorSetLayoutBinding {
//             .binding = 0U, // TODO: ShaderGlobals::Binding,
//             .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
//             .descriptorCount = 1,
//             .stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS,
//             .pImmutableSamplers = nullptr,
//         },
//         VkDescriptorSetLayoutBinding {
//             .binding = 1U, //TODO: ShaderTransform::Binding,
//             .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
//             .descriptorCount = 1,
//             .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
//             .pImmutableSamplers = nullptr,
//         },
//         // TODO: ShaderMaterial
//     };

//     _vkDescriptorSetLayoutList.resize(1, VK_NULL_HANDLE);

//     VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = {
//         .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
//         .pNext = nullptr,
//         .flags = 0,
//         .bindingCount = static_cast<uint32_t>(descriptorSetLayoutBindingList.size()),
//         .pBindings = descriptorSetLayoutBindingList.data(),
//     };

//     vkResult = vkCreateDescriptorSetLayout(
//         Device,
//         &descriptorSetLayoutCreateInfo,
//         nullptr,
//         &_vkDescriptorSetLayoutList[0]
//     );

//     if (vkResult != VK_SUCCESS) {
//         throw Exception("vkCreateDescriptorSetLayout() failed");
//     }
// }

// void termDescriptorPool()
// {
//     for (auto& descriptorSetLayout : _vkDescriptorSetLayoutList) {
//         if (descriptorSetLayout) {
//             vkDestroyDescriptorSetLayout(Device, descriptorSetLayout, nullptr);
//             descriptorSetLayout = VK_NULL_HANDLE;
//         }
//     }

//     if (_vkDescriptorPool) {
//         vkDestroyDescriptorPool(Device, _vkDescriptorPool, nullptr);
//         _vkDescriptorPool = VK_NULL_HANDLE;
//     }
// }


RYME_API
Tuple<vk::Buffer, VmaAllocation> CreateBuffer(
    vk::BufferCreateInfo& bufferCreateInfo,
    VmaAllocationCreateInfo& allocationCreateInfo,
    VmaAllocationInfo * allocationInfo /*= nullptr*/
)
{
    vk::Buffer buffer;
    VmaAllocation allocation;
    
    auto vkResult = (vk::Result)vmaCreateBuffer(
        Allocator,
        reinterpret_cast<const VkBufferCreateInfo *>(&bufferCreateInfo),
        &allocationCreateInfo,
        reinterpret_cast<VkBuffer *>(&buffer),
        &allocation,
        allocationInfo
    );

    vk::resultCheck(vkResult, "vmaCreateBuffer");

    return { buffer, allocation };
}

RYME_API
Tuple<vk::Image, VmaAllocation> CreateImage(
    vk::ImageCreateInfo& imageCreateInfo,
    VmaAllocationCreateInfo& allocationCreateInfo,
    VmaAllocationInfo * allocationInfo /*= nullptr*/
)
{
    vk::Image image;
    VmaAllocation allocation;
    
    auto vkResult = (vk::Result)vmaCreateImage(
        Allocator,
        reinterpret_cast<const VkImageCreateInfo *>(&imageCreateInfo),
        &allocationCreateInfo,
        reinterpret_cast<VkImage *>(&image),
        &allocation,
        allocationInfo
    );

    vk::resultCheck(vkResult, "vmaCreateImage");

    return { image, allocation };
}

RYME_API
void CopyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::BufferCopy region)
{
    auto allocateInfo = vk::CommandBufferAllocateInfo()
        .setCommandPool(_commandPool)
        .setLevel(vk::CommandBufferLevel::ePrimary)
        .setCommandBufferCount(1);

    auto commandBufferList = Device.allocateCommandBuffers(allocateInfo);
    auto commandBuffer = commandBufferList.front();

    auto beginInfo = vk::CommandBufferBeginInfo()
        .setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

    commandBuffer.begin(beginInfo);

    commandBuffer.copyBuffer(srcBuffer, dstBuffer, 1, &region);

    commandBuffer.end();

    auto submitInfo = vk::SubmitInfo()
        .setCommandBuffers(commandBufferList);

    _graphicsQueue.submit({ submitInfo }, nullptr);

    _graphicsQueue.waitIdle();

    Device.freeCommandBuffers(_commandPool, commandBufferList);
}

RYME_API
void CopyBufferToImage(vk::Buffer src, vk::Image dst, vk::BufferImageCopy region)
{
    auto allocateInfo = vk::CommandBufferAllocateInfo()
        .setCommandPool(_commandPool)
        .setLevel(vk::CommandBufferLevel::ePrimary)
        .setCommandBufferCount(1);

    auto commandBufferList = Device.allocateCommandBuffers(allocateInfo);
    auto commandBuffer = commandBufferList.front();

    auto subresourceRange = vk::ImageSubresourceRange()
        .setAspectMask(region.imageSubresource.aspectMask)
        .setBaseMipLevel(0)
        .setLevelCount(region.imageSubresource.mipLevel + 1)
        .setBaseArrayLayer(region.imageSubresource.baseArrayLayer)
        .setLayerCount(region.imageSubresource.layerCount);
    
    auto barrier = vk::ImageMemoryBarrier()
        .setOldLayout(vk::ImageLayout::eUndefined)
        .setNewLayout(vk::ImageLayout::eTransferDstOptimal)
        .setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
        .setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
        .setImage(dst)
        .setSubresourceRange(subresourceRange)
        .setSrcAccessMask({})
        .setDstAccessMask(vk::AccessFlagBits::eTransferWrite);

    auto beginInfo = vk::CommandBufferBeginInfo()
        .setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

    commandBuffer.begin(beginInfo);

    // TODO: Investigate
    commandBuffer.pipelineBarrier(
        vk::PipelineStageFlagBits::eTopOfPipe,
        vk::PipelineStageFlagBits::eTransfer,
        {},
        0, nullptr,
        0, nullptr,
        1, &barrier
    );

    commandBuffer.copyBufferToImage(
        src,
        dst,
        vk::ImageLayout::eTransferDstOptimal,
        1, &region
    );

    commandBuffer.end();

    auto submitInfo = vk::SubmitInfo()
        .setCommandBuffers(commandBufferList);

    _graphicsQueue.submit({ submitInfo }, nullptr);

    _graphicsQueue.waitIdle();

    Device.freeCommandBuffers(_commandPool, commandBufferList);
}

} // namespace Graphics

} // namespace ryme