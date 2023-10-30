#ifndef RYME_GRAPHICS_HPP
#define RYME_GRAPHICS_HPP

#include <Ryme/Config.hpp>
#include <Ryme/InitInfo.hpp>
#include <Ryme/Math.hpp>
#include <Ryme/String.hpp>
#include <Ryme/Tuple.hpp>

#include <Ryme/ThirdParty/python.hpp>
#include <Ryme/ThirdParty/SDL.hpp>
#include <Ryme/ThirdParty/vulkan.hpp>
#include <vulkan/vulkan_core.h>

namespace ryme {

///
/// Vulkan Graphics System
///
namespace Graphics {

extern vk::Instance Instance;

extern vk::Device Device;

extern VmaAllocator Allocator;

extern vk::RenderPass RenderPass;

RYME_API
void Init(const InitInfo& initInfo);

RYME_API
void Term();

RYME_API
void Render();

RYME_API
void HandleEvent(SDL_Event& event);

RYME_API
void SetWindowTitle(String windowTitle);

RYME_API
String GetWindowTitle();

RYME_API
void SetWindowSize(Vec2i windowSize);

RYME_API
Vec2i GetWindowSize();

RYME_API
Tuple<vk::Buffer, VmaAllocation> CreateBuffer(
    vk::BufferCreateInfo& bufferCreateInfo,
    VmaAllocationCreateInfo& allocationCreateInfo,
    VmaAllocationInfo * allocationInfo = nullptr
);

RYME_API
Tuple<vk::Image, VmaAllocation> CreateImage(
    vk::ImageCreateInfo& imageCreateInfo,
    VmaAllocationCreateInfo& allocationCreateInfo,
    VmaAllocationInfo * allocationInfo = nullptr
);

RYME_API
void CopyBuffer(vk::Buffer src, vk::Buffer dst, vk::BufferCopy region);

RYME_API
void CopyBufferToImage(vk::Buffer src, vk::Image dst, vk::BufferImageCopy region);

RYME_API
void ScriptInit(py::module);

RYME_API
void SetRenderFunc(std::function<void(vk::CommandBuffer)> func);

} // namespace Graphics

} // namespace ryme

#endif // RYME_GRAPHICS_HPP