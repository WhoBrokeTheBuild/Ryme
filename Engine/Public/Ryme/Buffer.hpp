#ifndef RYME_BUFFER_HPP
#define RYME_BUFFER_HPP

#include <Ryme/Config.hpp>
#include <Ryme/NonCopyable.hpp>

#include <Ryme/ThirdParty/vulkan.hpp>

namespace ryme {

class RYME_API Buffer : public NonCopyable
{
public:

    Buffer() = default;
    
    Buffer(Buffer&&) = default;

    virtual ~Buffer();

    void Create(vk::DeviceSize size, uint8_t * data, vk::BufferUsageFlagBits bufferUsage, VmaMemoryUsage memoryUsage);

    void Destroy();

    inline vk::DeviceSize GetSize() const {
        return _size;
    }

    inline vk::BufferUsageFlags GetBufferUsage() const {
        return _bufferUsage;
    }

    inline VmaMemoryUsage GetMemoryUsage() const {
        return _memoryUsage;
    }

    inline bool IsMapped() const {
        return (_mappedBufferMemory != nullptr);
    }

    virtual void ReadFrom(size_t offset, size_t length, uint8_t * data);

    virtual void WriteTo(size_t offset, size_t length, uint8_t * data);

    inline vk::Buffer GetVkBuffer() {
        return _buffer;
    }

private:

    vk::DeviceSize _size;

    vk::BufferUsageFlags _bufferUsage;

    VmaMemoryUsage _memoryUsage;

    vk::Buffer _buffer = nullptr;
    
    VmaAllocation _allocation = nullptr;

    uint8_t * _mappedBufferMemory = nullptr;

}; // class Buffer

} // namespace ryme

#endif // RYME_BUFFER_HPP