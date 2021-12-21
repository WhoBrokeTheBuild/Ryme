#include <Ryme/Buffer.hpp>
#include <Ryme/Graphics.hpp>
#include <Ryme/Exception.hpp>

namespace ryme {

Buffer::~Buffer()
{
    Destroy();
}

void Buffer::Create(vk::DeviceSize size, uint8_t * data, vk::BufferUsageFlags bufferUsage, vma::MemoryUsage memoryUsage)
{
    VkResult vkResult;

    _size = size;
    _bufferUsage = bufferUsage;
    _memoryUsage = memoryUsage;

    // If we are uploading to a GPU only buffer, we need to use a staging buffer
    if (_memoryUsage == vma::MemoryUsage::eGpuOnly) {
        if (!data) {
            throw Exception("Attempting to create a GPU only buffer with no data");
        }

        auto stagingBufferCreateInfo = vk::BufferCreateInfo()
            .setSize(_size)
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

        memcpy(stagingAllocationInfo.pMappedData, data, _size);

        vk::BufferUsageFlags dstBufferUsage = vk::BufferUsageFlagBits::eTransferDst | _bufferUsage;

        auto bufferCreateInfo = vk::BufferCreateInfo()
            .setSize(_size)
            .setUsage(dstBufferUsage);

        auto allocationCreateInfo = vma::AllocationCreateInfo()
            .setUsage(_memoryUsage);

        std::tie(_buffer, _allocation) = Graphics::Allocator.createBuffer(
            bufferCreateInfo,
            allocationCreateInfo
        );

        auto region = vk::BufferCopy()
            .setSize(_size);

        Graphics::CopyBuffer(stagingBuffer, _buffer, region);

        Graphics::Allocator.freeMemory(stagingAllocation);

        Graphics::Device.destroyBuffer(stagingBuffer);
    }
    else {
        auto bufferCreateInfo = vk::BufferCreateInfo()
            .setSize(_size)
            .setUsage(_bufferUsage);

        auto allocationCreateInfo = vma::AllocationCreateInfo()
            .setFlags(vma::AllocationCreateFlagBits::eMapped)
            .setUsage(_memoryUsage);
        
        vma::AllocationInfo allocationInfo;

        std::tie(_buffer, _allocation) = Graphics::Allocator.createBuffer(
            bufferCreateInfo,
            allocationCreateInfo,
            allocationInfo
        );

        _mappedBufferMemory = reinterpret_cast<uint8_t *>(allocationInfo.pMappedData);

        if (data) {
            memcpy(_mappedBufferMemory, data, _size);
        }
    }
}

void Buffer::Destroy()
{
    Graphics::Allocator.unmapMemory(_allocation);
    _mappedBufferMemory = nullptr;

    Graphics::Device.destroyBuffer(_buffer);

    Graphics::Allocator.freeMemory(_allocation);

    _size = 0;
}

void Buffer::ReadFrom(size_t offset, size_t length, uint8_t * data)
{
    assert(_memoryUsage == vma::MemoryUsage::eGpuToCpu);
    assert(_mappedBufferMemory);

    memcpy(data, _mappedBufferMemory + offset, length);
}

void Buffer::WriteTo(size_t offset, size_t length, uint8_t * data)
{
    assert(_memoryUsage == vma::MemoryUsage::eCpuOnly
        || _memoryUsage == vma::MemoryUsage::eCpuToGpu);
    assert(_mappedBufferMemory);

    memcpy(_mappedBufferMemory + offset, data, length);
}

} // namespace ryme