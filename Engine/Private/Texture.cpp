#include <Ryme/Texture.hpp>
#include <Ryme/Graphics.hpp>
#include <Ryme/Log.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace ryme {
    
Texture::Texture(const Path& path, vk::SamplerCreateInfo samplerCreateInfo /*= {}*/, bool search /*= true*/)
{
    LoadFromFile(path, samplerCreateInfo, search);
}

bool Texture::LoadFromFile(const Path& path, vk::SamplerCreateInfo samplerCreateInfo /*= {}*/, bool search /*= true*/)
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
    
    vk::DeviceSize size = width * height * STBI_rgb_alpha;

    auto stagingBufferCreateInfo = vk::BufferCreateInfo()
        .setSize(size)
        .setUsage(vk::BufferUsageFlagBits::eTransferSrc);
    
    auto stagingAllocationCreateInfo = vma::AllocationCreateInfo()
        .setFlags(vma::AllocationCreateFlagBits::eMapped)
        .setUsage(vma::MemoryUsage::eCpuOnly);

    vma::AllocationInfo stagingAllocationInfo;

    auto[stagingBuffer, stagingAllocation] = Graphics::Allocator.createBuffer(
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
        .setInitialLayout(vk::ImageLayout::eUndefined) // Preinitialized?
        .setSamples(vk::SampleCountFlagBits::e1);

    auto allocationCreateInfo = vma::AllocationCreateInfo()
        .setUsage(vma::MemoryUsage::eGpuOnly);

    std::tie(_image, _allocation) = Graphics::Allocator
        .createImage(imageCreateInfo, allocationCreateInfo);

    auto subresourceLayers = vk::ImageSubresourceLayers()
        .setAspectMask(vk::ImageAspectFlagBits::eColor)
        .setMipLevel(0)
        .setBaseArrayLayer(0)
        .setLayerCount(1);

    auto region = vk::BufferImageCopy()
        .setBufferOffset(0)
        .setBufferRowLength(0)
        .setBufferImageHeight(0)
        .setImageSubresource(subresourceLayers)
        .setImageOffset(vk::Offset3D(0, 0, 0))
        .setImageExtent(vk::Extent3D(width, height, 1));

    Graphics::CopyBufferToImage(stagingBuffer, _image, region);

    Graphics::Allocator.freeMemory(stagingAllocation);

    Graphics::Device.destroyBuffer(stagingBuffer);

    auto subresourceRange = vk::ImageSubresourceRange()
        .setAspectMask(vk::ImageAspectFlagBits::eColor)
        .setBaseMipLevel(0)
        .setLevelCount(1)
        .setBaseArrayLayer(0)
        .setLayerCount(1);

    auto imageViewCreateInfo = vk::ImageViewCreateInfo()
        .setImage(_image)
        .setViewType(vk::ImageViewType::e2D)
        .setFormat(vk::Format::eR8G8B8A8Srgb)
        .setSubresourceRange(subresourceRange);

    _imageView = Graphics::Device.createImageView(imageViewCreateInfo);

    _sampler = Graphics::Device.createSampler(samplerCreateInfo);

    _path = fullPath;
    _samplerCreateInfo = samplerCreateInfo;
    
    Log(RYME_ANCHOR, "Loaded '{}'", fullPath);

    _isLoaded = true;
    return true;
}

void Texture::Free()
{
    Graphics::Device.destroySampler(_sampler);

    Graphics::Device.destroyImageView(_imageView);

    Graphics::Device.destroyImage(_image);

    Graphics::Allocator.freeMemory(_allocation);
}

bool Texture::Reload()
{
    return LoadFromFile(_path, _samplerCreateInfo, false);
}

} // namespace ryme