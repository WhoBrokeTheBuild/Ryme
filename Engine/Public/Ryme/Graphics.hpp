#ifndef RYME_GRAPHICS_HPP
#define RYME_GRAPHICS_HPP

#include <Ryme/Config.hpp>
#include <Ryme/Math.hpp>
#include <Ryme/String.hpp>

#include <Ryme/ThirdParty/python.hpp>
#include <Ryme/ThirdParty/SDL.hpp>
#include <Ryme/ThirdParty/vulkan.hpp>

namespace ryme {

///
/// Vulkan Graphics System
///
namespace Graphics {

RYME_API
void ScriptInit(py::module);

RYME_API
void Init(String windowTitle, Vec2i windowSize);

RYME_API
void Term();

RYME_API
void SetWindowTitle(String windowTitle);

RYME_API
String GetWindowTitle();

RYME_API
void SetWindowSize(Vec2i windowSize);

RYME_API
Vec2i GetWindowSize();

RYME_API
vk::Instance& GetVkInstance();

RYME_API
vk::Device& GetVkDevice();

RYME_API
vma::Allocator& GetVmaAllocator();

RYME_API
void CopyBuffer(vk::Buffer src, vk::Buffer dst, vk::BufferCopy region);

RYME_API
void CopyBufferToImage(vk::Buffer src, vk::Image dst, vk::BufferImageCopy region);

} // namespace Graphics

} // namespace ryme

#endif // RYME_GRAPHICS_HPP