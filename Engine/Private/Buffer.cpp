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
    _vkBufferUsage = bufferUsage;
    _vmaMemoryUsage = memoryUsage;

    auto& vkDevice = Graphics::GetVkDevice();
    auto& vmaAllocator = Graphics::GetVmaAllocator();

    // If we are uploading to a GPU only buffer, we need to use a staging buffer
    if (_vmaMemoryUsage == vma::MemoryUsage::eGpuOnly) {
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

        auto[stagingBuffer, stagingAllocation] = vmaAllocator.createBuffer(
            stagingBufferCreateInfo,
            stagingAllocationCreateInfo,
            stagingAllocationInfo
        );

        memcpy(stagingAllocationInfo.pMappedData, data, _size);

        vk::BufferUsageFlags dstBufferUsage = vk::BufferUsageFlagBits::eTransferDst | _vkBufferUsage;

        auto bufferCreateInfo = vk::BufferCreateInfo()
            .setSize(_size)
            .setUsage(dstBufferUsage);

        auto allocationCreateInfo = vma::AllocationCreateInfo()
            .setUsage(_vmaMemoryUsage);

        std::tie(_vkBuffer, _vmaAllocation) = vmaAllocator.createBuffer(
            bufferCreateInfo,
            allocationCreateInfo
        );

        auto region = vk::BufferCopy()
            .setSize(_size);

        Graphics::CopyBuffer(stagingBuffer, _vkBuffer, region);

        vmaAllocator.freeMemory(stagingAllocation);

        vkDevice.destroyBuffer(stagingBuffer);
    }
    else {
        auto bufferCreateInfo = vk::BufferCreateInfo()
            .setSize(_size)
            .setUsage(_vkBufferUsage);

        auto allocationCreateInfo = vma::AllocationCreateInfo()
            .setFlags(vma::AllocationCreateFlagBits::eMapped)
            .setUsage(_vmaMemoryUsage);
        
        vma::AllocationInfo allocationInfo;

        std::tie(_vkBuffer, _vmaAllocation) = vmaAllocator.createBuffer(
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
    auto& vkDevice = Graphics::GetVkDevice();
    auto& vmaAllocator = Graphics::GetVmaAllocator();

    vmaAllocator.unmapMemory(_vmaAllocation);
    _mappedBufferMemory = nullptr;

    vkDevice.destroyBuffer(_vkBuffer);

    vmaAllocator.freeMemory(_vmaAllocation);

    _size = 0;
}

void Buffer::ReadFrom(size_t offset, size_t length, uint8_t * data)
{
    assert(_vmaMemoryUsage == vma::MemoryUsage::eGpuToCpu);
    assert(_mappedBufferMemory);

    memcpy(data, _mappedBufferMemory + offset, length);
}

void Buffer::WriteTo(size_t offset, size_t length, uint8_t * data)
{
    assert(_vmaMemoryUsage == vma::MemoryUsage::eCpuOnly
        || _vmaMemoryUsage == vma::MemoryUsage::eCpuToGpu);
    assert(_mappedBufferMemory);

    memcpy(_mappedBufferMemory + offset, data, length);
}

} // namespace ryme