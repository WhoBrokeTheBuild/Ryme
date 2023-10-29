#include <Ryme/Buffer.hpp>
#include <Ryme/Graphics.hpp>
#include <Ryme/Exception.hpp>

namespace ryme {

RYME_API
Buffer::~Buffer()
{
    Destroy();
}

RYME_API
void Buffer::Create(vk::DeviceSize size, uint8_t * data, vk::BufferUsageFlagBits bufferUsage, VmaMemoryUsage memoryUsage)
{
    _size = size;
    _bufferUsage = bufferUsage;
    _memoryUsage = memoryUsage;

    if (_memoryUsage == VMA_MEMORY_USAGE_GPU_ONLY) {
        if (not data) {
            throw Exception("Attempting to create a GPU only buffer with no data");
        }

        auto stagingBufferCreateInfo = vk::BufferCreateInfo()
            .setSize(_size)
            .setUsage(vk::BufferUsageFlagBits::eTransferSrc)
            .setSharingMode(vk::SharingMode::eExclusive);
        
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

        memcpy(stagingAllocationInfo.pMappedData, data, _size);

        auto bufferCreateInfo = vk::BufferCreateInfo()
            .setSize(_size)
            .setUsage(vk::BufferUsageFlagBits::eTransferDst | _bufferUsage)
            .setSharingMode(vk::SharingMode::eExclusive);

        auto allocationCreateInfo = VmaAllocationCreateInfo{
            .usage = _memoryUsage,
        };
        
        VmaAllocationInfo allocationInfo;

        std::tie(_buffer, _allocation) = Graphics::CreateBuffer(
            bufferCreateInfo,
            allocationCreateInfo,
            &allocationInfo
        );

        auto region = vk::BufferCopy()
            .setSize(_size);

        Graphics::CopyBuffer(stagingBuffer, _buffer, region);

        vmaFreeMemory(Graphics::Allocator, stagingAllocation);
        
        Graphics::Device.destroyBuffer(stagingBuffer);
    }
    else {
        auto bufferCreateInfo = vk::BufferCreateInfo()
            .setSize(_size)
            .setUsage(_bufferUsage);

        auto allocationCreateInfo = VmaAllocationCreateInfo{
            .flags = VMA_ALLOCATION_CREATE_MAPPED_BIT,
            .usage = _memoryUsage,
        };
        
        VmaAllocationInfo allocationInfo;

        std::tie(_buffer, _allocation) = Graphics::CreateBuffer(
            bufferCreateInfo,
            allocationCreateInfo,
            &allocationInfo
        );

        _mappedBufferMemory = reinterpret_cast<uint8_t *>(allocationInfo.pMappedData);

        if (data) {
            memcpy(_mappedBufferMemory, data, _size);
        }
    }
}

RYME_API
void Buffer::Destroy()
{
    _size = 0;

    if (_mappedBufferMemory) {
        vmaUnmapMemory(Graphics::Allocator, _allocation);
        _mappedBufferMemory = nullptr;
    }

    Graphics::Device.destroyBuffer(_buffer);
    _buffer = nullptr;

    vmaFreeMemory(Graphics::Allocator, _allocation);
    _allocation = nullptr;
}

RYME_API
void Buffer::ReadFrom(size_t offset, size_t length, uint8_t * data)
{
    assert(_memoryUsage == VMA_MEMORY_USAGE_GPU_TO_CPU);
    assert(_mappedBufferMemory);

    memcpy(data, _mappedBufferMemory + offset, length);
}

RYME_API
void Buffer::WriteTo(size_t offset, size_t length, uint8_t * data)
{
    assert(_memoryUsage == VMA_MEMORY_USAGE_CPU_ONLY
        or _memoryUsage == VMA_MEMORY_USAGE_CPU_TO_GPU);
    assert(_mappedBufferMemory);

    memcpy(_mappedBufferMemory + offset, data, length);
}

} // namespace ryme