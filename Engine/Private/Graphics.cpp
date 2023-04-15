#include <Ryme/Graphics.hpp>
#include <Ryme/Ryme.hpp>
#include <Ryme/Buffer.hpp>
#include <Ryme/Shader.hpp>

#include <Ryme/ShaderGlobals.hpp>
#include <Ryme/ShaderTransform.hpp>

#include <SDL_vulkan.h>

RYME_DISABLE_WARNINGS()

    #define VMA_IMPLEMENTATION
    #include <vk_mem_alloc.h>

RYME_ENABLE_WARNINGS()

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

namespace ryme {

namespace Graphics {

// Window

SDL_Window * _window = nullptr;

Vec2i _windowSize;

String _windowTitle;

Vec4 _clearColor;

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

vk::Extent2D _swapchainExtent;

vk::SwapchainKHR _swapchain;

vk::Format _swapchainImageFormat;

vk::ColorSpaceKHR _swapchainColorSpace;

List<vk::Image> _swapchainImageList;

List<vk::ImageView> _swapchainImageViewList;

List<VkFramebuffer> _framebufferList;

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

unsigned _currentFrame;

List<vk::Semaphore> _imageAvailableSemaphoreList;

List<vk::Semaphore> _renderingFinishedSemaphoreList;

List<vk::Fence> _inFlightFenceList;

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
        SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE
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
                  vk::DebugUtilsMessageSeverityFlagBitsEXT::eError
                | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning
                // | vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo
            )
            .setMessageType(
                  vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral
                | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation
                | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance
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
        .setExtent(vk::Extent3D(_swapchainExtent, 1))
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
        .setFormat(_swapchainImageFormat)
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
    // Buffer shaderGlobalsBuffer;
    // shaderGlobalsBuffer.Create(
    //     sizeof(ShaderGlobals),
    //     nullptr,
    //     vk::BufferUsageFlagBits::eUniformBuffer,
    //     VMA_MEMORY_USAGE_CPU_TO_GPU
    // );

}

void initFramebufferList()
{
    for (auto& framebuffer : _framebufferList) {
        Device.destroyFramebuffer(framebuffer);
        framebuffer = nullptr;
    }

    size_t backbufferCount = _swapchainImageViewList.size();

    _framebufferList.resize(_swapchainImageViewList.size());

    for (unsigned i = 0; i < backbufferCount; ++i) {
        Array<vk::ImageView, 2> attachmentList = {
            _swapchainImageViewList[i],
            _depthImageView,
        };

        auto framebufferCreateInfo = vk::FramebufferCreateInfo()
            .setRenderPass(RenderPass)
            .setAttachments(attachmentList)
            .setWidth(_swapchainExtent.width)
            .setHeight(_swapchainExtent.height)
            .setLayers(1);

        _framebufferList[i] = Device.createFramebuffer(framebufferCreateInfo);
    }
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
            _swapchainImageList.size()
        )
    );
}

void initSyncObjects()
{
    for (auto fence : _inFlightFenceList) {
        Device.destroyFence(fence);
        fence = nullptr;
    }

    for (auto& semaphore : _imageAvailableSemaphoreList) {
        Device.destroySemaphore(semaphore);
        semaphore = nullptr;
    }

    for (auto& semaphore : _renderingFinishedSemaphoreList) {
        Device.destroySemaphore(semaphore);
        semaphore = nullptr;
    }

    size_t backbufferCount = _swapchainImageViewList.size();

    _imageAvailableSemaphoreList.resize(backbufferCount);
    _renderingFinishedSemaphoreList.resize(backbufferCount);
    _inFlightFenceList.resize(backbufferCount);

    auto semaphoreCreateInfo = vk::SemaphoreCreateInfo();

    auto fenceCreateInfo = vk::FenceCreateInfo()
        .setFlags(vk::FenceCreateFlagBits::eSignaled);

    for (unsigned i = 0; i < backbufferCount; ++i) {
        _imageAvailableSemaphoreList[i] = Device.createSemaphore(semaphoreCreateInfo);
        _renderingFinishedSemaphoreList[i] = Device.createSemaphore(semaphoreCreateInfo);
        _inFlightFenceList[i] = Device.createFence(fenceCreateInfo);
    }
}

void fillCommandBuffers()
{
    for (size_t i = 0; i < _commandBufferList.size(); ++i) {
        auto& commandBuffer = _commandBufferList[i];

        auto commandBufferBeginInfo = vk::CommandBufferBeginInfo();
        commandBuffer.begin(commandBufferBeginInfo);

        Vec4 clearColor = _clearColor;

        // If our surface is sRGB, Vulkan will try to convert our color to sRGB
        // This fails and washes out the color, so we convert to linear to account for it        
        if (_swapchainColorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
            clearColor = Color::ToLinear(clearColor);
        }

        auto clearValueArray = Color::ToArray(clearColor);

        Array<vk::ClearValue, 2> clearValueList = {
            vk::ClearValue(clearValueArray),
            vk::ClearValue({ 1.0f, 0 }),
        };

        auto renderArea = vk::Rect2D()
            .setOffset({ 0, 0 })
            .setExtent(_swapchainExtent);

        auto renderPassBeginInfo = vk::RenderPassBeginInfo()
            .setRenderPass(RenderPass)
            .setFramebuffer(_framebufferList[i])
            .setRenderArea(renderArea)
            .setClearValues(clearValueList);

        commandBuffer.beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);

            // commandBuffer.

        commandBuffer.endRenderPass();

        commandBuffer.end();
    }
}

void initSwapchain()
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
        bool isPreferredFormat = (
            format.format == vk::Format::eR8G8B8A8Srgb ||
            format.format == vk::Format::eB8G8R8A8Srgb
        );

        bool isSRGB = (format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear);
        
        if (isPreferredFormat && isSRGB) {
            imageFormat = format;
        }

        Log(RYME_ANCHOR, "\t{} {}",
            vk::to_string(format.format),
            vk::to_string(format.colorSpace)
        );
    }

    Log(RYME_ANCHOR, "Vulkan Swap Chain Image Format: {} {} ",
        vk::to_string(imageFormat.format),
        vk::to_string(imageFormat.colorSpace)
    );

    _swapchainImageFormat = imageFormat.format;
    _swapchainColorSpace = imageFormat.colorSpace;

    /// Image Extent

    auto surfaceCapabilities = _physicalDevice.getSurfaceCapabilitiesKHR(_surface);

    _swapchainExtent = surfaceCapabilities.currentExtent;

    if (_swapchainExtent.width == UINT32_MAX) {
        _swapchainExtent.width = std::clamp(
            static_cast<uint32_t>(_windowSize.x),
            surfaceCapabilities.minImageExtent.width,
            surfaceCapabilities.maxImageExtent.width
        );

        _swapchainExtent.height = std::clamp(
            static_cast<uint32_t>(_windowSize.y),
            surfaceCapabilities.minImageExtent.height,
            surfaceCapabilities.maxImageExtent.height
        );
    }

    Log(RYME_ANCHOR, "Vulkan Swap Chain Extent: {}x{}",
        _swapchainExtent.width,
        _swapchainExtent.height
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

    auto oldSwapChain = _swapchain;

    auto swapChainCreateInfo = vk::SwapchainCreateInfoKHR()
        .setSurface(_surface)
        .setMinImageCount(surfaceCapabilities.minImageCount)
        .setImageFormat(imageFormat.format)
        .setImageColorSpace(imageFormat.colorSpace)
        .setImageExtent(_swapchainExtent)
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

    _swapchain = Device.createSwapchainKHR(swapChainCreateInfo);

    Device.destroySwapchainKHR(oldSwapChain);

    /// Image List

    _swapchainImageList = Device.getSwapchainImagesKHR(_swapchain);

    for (auto& imageView : _swapchainImageViewList) {
        Device.destroyImageView(imageView);
    }

    _swapchainImageViewList.clear();
    _swapchainImageViewList.reserve(_swapchainImageList.size());

    auto imageViewCreateInfo = vk::ImageViewCreateInfo()
        .setViewType(vk::ImageViewType::e2D)
        .setFormat(_swapchainImageFormat)
        .setSubresourceRange({ vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 });

    for (const auto& image : _swapchainImageList) {
        imageViewCreateInfo.setImage(image);
        _swapchainImageViewList.push_back(
            Device.createImageView(imageViewCreateInfo)
        );
    }

    initDepthBuffer();
    initRenderPass();
    initFramebufferList();
    initCommandBufferList();
    fillCommandBuffers(); // TODO:
    initSyncObjects();

    RYME_BENCHMARK_END();
}

RYME_API
void Init(const InitInfo& initInfo)
{
    RYME_BENCHMARK_START();

    _windowSize = initInfo.WindowSize;
    _windowTitle = initInfo.WindowTitle;
    _clearColor = initInfo.ClearColor;

    _currentFrame = 0;
    
    initWindow();
    initInstance();
    initSurface();
    initDevice();
    initAllocator();

    initSwapchain();
    initSwapchain(); // Test swap chain recreation

    
    RYME_BENCHMARK_END();
}

RYME_API
void Term()
{
    RYME_BENCHMARK_START();

    Device.waitIdle();

    // termSyncObjects

    for (auto fence : _inFlightFenceList) {
        Device.destroyFence(fence);
        fence = nullptr;
    }

    for (auto semaphore : _renderingFinishedSemaphoreList) {
        Device.destroySemaphore(semaphore);
        semaphore = nullptr;
    }

    for (auto semaphore : _imageAvailableSemaphoreList) {
        Device.destroySemaphore(semaphore);
        semaphore = nullptr;
    }

    // termCommandBufferList

    Device.freeCommandBuffers(_commandPool, _commandBufferList);

    Device.destroyCommandPool(_commandPool);

    // termFramebufferList

    for (auto framebuffer : _framebufferList) {
        Device.destroyFramebuffer(framebuffer);
        framebuffer = nullptr;
    }

    // termRenderPass

    Device.destroyRenderPass(RenderPass);

    // termDepthBuffer

    Device.destroyImageView(_depthImageView);
    
    Device.destroyImage(_depthImage);

    vmaFreeMemory(Allocator, _depthImageAllocation);

    // termSwapchain

    for (auto& imageView : _swapchainImageViewList) {
        Device.destroyImageView(imageView);
        imageView = nullptr;
    }

    // The vk::Image's in _swapchainImageList are destroyed as well
    Device.destroySwapchainKHR(_swapchain);

    // termAllocator

    vmaDestroyAllocator(Allocator);

    // termDevice

    Device.destroy();

    // termInstance

    Instance.destroySurfaceKHR(_surface);

    Instance.destroyDebugUtilsMessengerEXT(_debugUtilsMessenger);

    Instance.destroy();

    // termWindow

    if (_window) {
        SDL_DestroyWindow(_window);
        _window = nullptr;
    }

    SDL_Quit();

    RYME_BENCHMARK_END();
}

RYME_API
void Render()
{
    static bool updateSwapchain = false;

    if (updateSwapchain) {
        Log(RYME_ANCHOR, "Regenerating Swapchain");
        initSwapchain();
        updateSwapchain = false;
    }

    constexpr uint64_t MaxTimeout = std::numeric_limits<uint64_t>::max();

    vk::Result vkResult;

    vmaSetCurrentFrameIndex(Allocator, _currentFrame);

    vkResult = Device.waitForFences(1, &_inFlightFenceList[_currentFrame], true, MaxTimeout);
    vk::resultCheck(vkResult, "vk::Device::waitForFences");

    vkResult = Device.resetFences(1, &_inFlightFenceList[_currentFrame]);

    uint32_t imageIndex;

    // We explicitly invoke the nothrow version of this function, since the enhanced version throws vk::OutOfDateKHRError
    // https://github.com/KhronosGroup/Vulkan-Hpp/issues/599
    vkResult = Device.acquireNextImageKHR(
        _swapchain,
        MaxTimeout,
        _imageAvailableSemaphoreList[_currentFrame],
        nullptr,
        &imageIndex
    );

    if (vkResult == vk::Result::eErrorOutOfDateKHR) {
        updateSwapchain = true;
        return;
    }

    vk::resultCheck(vkResult, "vk::Device::acquireNextImageKHR",
        { vk::Result::eSuccess, vk::Result::eSuboptimalKHR });

    Array<vk::Semaphore, 1> waitSemaphoreList = {
        _imageAvailableSemaphoreList[_currentFrame],
    };

    Array<vk::Semaphore, 1> signalSemaphoreList = {
        _renderingFinishedSemaphoreList[_currentFrame],
    };

    Array<vk::PipelineStageFlags, 1> waitStageList = {
        vk::PipelineStageFlagBits::eColorAttachmentOutput,
    };

    Array<vk::CommandBuffer, 1> commandBufferList = {
        _commandBufferList[imageIndex],
    };

    auto submitInfo = vk::SubmitInfo()
        .setWaitSemaphores(waitSemaphoreList)
        .setWaitDstStageMask(waitStageList)
        .setCommandBuffers(commandBufferList)
        .setSignalSemaphores(signalSemaphoreList);

    _graphicsQueue.submit(submitInfo, _inFlightFenceList[_currentFrame]);

    Array<vk::SwapchainKHR, 1> swapchainList = {
        _swapchain,
    };

    Array<uint32_t, 1> imageIndexList = {
        imageIndex,
    };

    auto presentInfo = vk::PresentInfoKHR()
        .setSwapchains(swapchainList)
        .setWaitSemaphores(signalSemaphoreList)
        .setImageIndices(imageIndexList);

    // We explicitly invoke the nothrow version of this function, since the enhanced version throws vk::OutOfDateKHRError
    // https://github.com/KhronosGroup/Vulkan-Hpp/issues/599
    // https://github.com/KhronosGroup/Vulkan-Hpp/issues/599
    vkResult = _presentQueue.presentKHR(&presentInfo);
    if (vkResult == vk::Result::eErrorOutOfDateKHR) {
        updateSwapchain = true;
        return;
    }

    vk::resultCheck(vkResult, "vk::Queue::presentKHR",
        { vk::Result::eSuccess, vk::Result::eSuboptimalKHR });

    _currentFrame = (_currentFrame + 1) % _inFlightFenceList.size();
}

RYME_API
void HandleEvent(SDL_Event& event)
{
    if (event.type == SDL_WINDOWEVENT) {
        if (event.window.type == SDL_WINDOWEVENT_RESIZED) {
            _windowSize = { event.window.data1, event.window.data2 };
        }
    }
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

} // namespace Graphics

} // namespace ryme