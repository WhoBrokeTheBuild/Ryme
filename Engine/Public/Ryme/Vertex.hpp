#ifndef RYME_VERTEX_HPP
#define RYME_VERTEX_HPP

#include <Ryme/Config.hpp>
#include <Ryme/Containers.hpp>
#include <Ryme/Math.hpp>

#include <Ryme/ThirdParty/vulkan.hpp>

namespace ryme {

struct Vertex
{
    struct AttributeLocation
    {
        static inline uint32_t Position  = 0;
        static inline uint32_t Normal    = 1;
        static inline uint32_t Tangent   = 2;
        static inline uint32_t Color     = 3;
        static inline uint32_t TexCoord1 = 4;
        static inline uint32_t TexCoord2 = 5;
        static inline uint32_t Joints    = 6;
        static inline uint32_t Weights   = 7;
    };

    Vec4 Position;

    Vec4 Normal;

    Vec4 Tangent;
    
    Vec4 Color;

    Vec2 TexCoord1;

    Vec2 TexCoord2;

    Vec2u Joints;

    Vec4 Weights;
    
}; // struct Vertex

static_assert(
    sizeof(Vertex) == 104,
    "sizeof(Vertex) is not tightly packed"
);

Span<vk::VertexInputBindingDescription> GetVertexInputBindingDescriptionList();

Span<vk::VertexInputAttributeDescription> GetVertexInputAttributeDescriptionList();

} // namespace ryme

#endif // RYME_VERTEX_HPP
