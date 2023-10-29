#include <Ryme/Texture.hpp>
#include <Ryme/Graphics.hpp>
#include <Ryme/Log.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace ryme {
    
RYME_API
Texture::Texture(const Path& path, vk::SamplerCreateInfo samplerCreateInfo /*= {}*/, bool search /*= true*/)
{
    LoadFromFile(path, samplerCreateInfo, search);
}

RYME_API
Texture::~Texture()
{
    Free();
}

RYME_API
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

    if (not data) {
        return false;
    }

    _path = fullPath;
    
    vk::DeviceSize size = width * height * STBI_rgb_alpha;

    auto stagingBufferCreateInfo = vk::BufferCreateInfo()
        .setSize(size)
        .setUsage(vk::BufferUsageFlagBits::eTransferSrc);
    
    auto stagingAllocationCreateInfo = VmaAllocationCreateInfo{
        .flags = VMA_ALLOCATION_CREATE_MAPPED_BIT,
        .usage = VMA_MEMORY_USAGE_CPU_ONLY,
    };

    VmaAllocationInfo stagingAllocationInfo;

    auto[stagingBuffer, stagingAllocation] = Graphics::CreateBuffer(
        stagingBufferCreateInfo,
        stagingAllocationCreateInfo,
        &stagingAllocationInfo
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
        .setSharingMode(vk::SharingMode::eExclusive)
        .setSamples(vk::SampleCountFlagBits::e1);

    auto allocationCreateInfo = VmaAllocationCreateInfo{
        .usage = VMA_MEMORY_USAGE_GPU_ONLY,
    };

    std::tie(_image, _allocation) = Graphics::CreateImage(
        imageCreateInfo,
        allocationCreateInfo
    );

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

    vmaFreeMemory(Graphics::Allocator, stagingAllocation);

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

    // TODO: Improve?
    _samplerCreateInfo = samplerCreateInfo;
    
    Log(RYME_ANCHOR, "Loaded '{}'", _path);

    _isLoaded = true;
    return true;
}

RYME_API
void Texture::Free()
{
    Graphics::Device.destroySampler(_sampler);
    _sampler = nullptr;

    Graphics::Device.destroyImageView(_imageView);
    _imageView = nullptr;

    Graphics::Device.destroyImage(_image);
    _image = nullptr;

    vmaFreeMemory(Graphics::Allocator, _allocation);
    _allocation = nullptr;
}

RYME_API
bool Texture::Reload()
{
    return LoadFromFile(_path, _samplerCreateInfo, false);
}

} // namespace ryme