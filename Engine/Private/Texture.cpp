#include <Ryme/Texture.hpp>
#include <Ryme/Graphics.hpp>
#include <Ryme/Log.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace ryme {
    
Texture::Texture(const Path& path, bool search /*= true*/)
{
    LoadFromFile(path, search);
}

bool Texture::LoadFromFile(const Path& path, bool search /*= true*/)
{
    int width;
    int height;
    int components;
    uint8_t * data = nullptr;

    Path fullPath = path;

    if (search) {
        for (const auto& assetPath : GetAssetPathList()) {
            fullPath = assetPath / path;

            data = stbi_load(fullPath.ToCString(), &width, &height, &components, STBI_rgb_alpha);
            if (data) {
                break;
            }
        }
    }
    else {
        data = stbi_load(path.ToCString(), &width, &height, &components, STBI_rgb_alpha);
    }

    if (!data) {
        return false;
    }
    
    auto& vkDevice = Graphics::GetVkDevice();
    auto& vmaAllocator = Graphics::GetVmaAllocator();

    vk::DeviceSize size = width * height * STBI_rgb_alpha;

    auto stagingBufferCreateInfo = vk::BufferCreateInfo()
        .setSize(size)
        .setUsage(vk::BufferUsageFlagBits::eTransferSrc);
    
    auto stagingAllocationCreateInfo = vma::AllocationCreateInfo()
        .setFlags(vma::AllocationCreateFlagBits::eMapped)
        .setUsage(vma::MemoryUsage::eCpuOnly);

    vma::AllocationInfo stagingAllocationInfo;

    auto[stagingBuffer, stagingAllocation] = vmaAllocator.createBuffer(
        stagingBufferCreateInfo,
        stagingAllocationCreateInfo,
        stagingAllocationInfo
    );
    
    memcpy(stagingAllocationInfo.pMappedData, data, size);

    stbi_image_free(data);

    auto imageCreateInfo = vk::ImageCreateInfo()
        .setImageType(vk::ImageType::e2D)
        .setFormat(vk::Format::eR8G8B8A8Srgb)
        .setTiling(vk::ImageTiling::eOptimal)
        .setExtent(vk::Extent3D(width, height, 1))
        .setMipLevels(1) // TODO: Investigate
        .setArrayLayers(1) // TODO: Investigate
        .setUsage(vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled)
        .setInitialLayout(vk::ImageLayout::eUndefined)
        .setSamples(vk::SampleCountFlagBits::e1);

    auto allocationCreateInfo = vma::AllocationCreateInfo()
        .setUsage(vma::MemoryUsage::eGpuOnly);

    std::tie(_vkImage, _vmaAllocation) = Graphics::GetVmaAllocator()
        .createImage(imageCreateInfo, allocationCreateInfo);

    auto imageSubresource = vk::ImageSubresourceLayers()
        .setAspectMask(vk::ImageAspectFlagBits::eColor)
        .setMipLevel(0)
        .setBaseArrayLayer(0)
        .setLayerCount(1);

    auto region = vk::BufferImageCopy()
        .setBufferOffset(0)
        .setBufferRowLength(0)
        .setBufferImageHeight(0)
        .setImageSubresource(imageSubresource)
        .setImageOffset(vk::Offset3D(0, 0, 0))
        .setImageExtent(vk::Extent3D(width, height, 1));

    Graphics::CopyBufferToImage(stagingBuffer, _vkImage, region);

    vmaAllocator.freeMemory(stagingAllocation);

    vkDevice.destroyBuffer(stagingBuffer);

    _path = fullPath;
    
    Log(RYME_ANCHOR, "Loaded '{}'", fullPath);

    _isLoaded = true;
    return true;
}

void Texture::Free()
{
    auto& vkDevice = Graphics::GetVkDevice();
    auto& vmaAllocator = Graphics::GetVmaAllocator();

    vkDevice.destroyImage(_vkImage);

    vmaAllocator.freeMemory(_vmaAllocation);
}

bool Texture::Reload()
{
    return LoadFromFile(_path, false);
}

} // namespace ryme