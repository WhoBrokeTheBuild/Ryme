#ifndef RYME_BUFFER_HPP
#define RYME_BUFFER_HPP

#include <Ryme/Config.hpp>

#include <Ryme/ThirdParty/vulkan.hpp>

namespace ryme {

class RYME_API Buffer
{
public:

    RYME_DISALLOW_COPY_AND_ASSIGN(Buffer)

    Buffer() = default;

    virtual ~Buffer();

    void Create(vk::DeviceSize size, uint8_t * data, vk::BufferUsageFlags bufferUsage, vma::MemoryUsage memoryUsage);

    void Destroy();

    inline vk::DeviceSize GetSize() const {
        return _size;
    }

    inline vk::BufferUsageFlags GetBufferUsage() const {
        return _bufferUsage;
    }

    inline vma::MemoryUsage GetMemoryUsage() const {
        return _memoryUsage;
    }

    inline bool IsMapped() const {
        return (_mappedBufferMemory != nullptr);
    }

    virtual void ReadFrom(size_t offset, size_t length, uint8_t * data);

    virtual void WriteTo(size_t offset, size_t length, uint8_t * data);

private:

    vk::DeviceSize _size;

    vk::BufferUsageFlags _bufferUsage;

    vma::MemoryUsage _memoryUsage;

    vk::Buffer _buffer;
    
    vma::Allocation _allocation;

    uint8_t * _mappedBufferMemory = nullptr;

}; // class Buffer

} // namespace ryme

#endif // RYME_BUFFER_HPP