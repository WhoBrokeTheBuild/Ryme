#ifndef RYME_VERTEX_HPP
#define RYME_VERTEX_HPP

#include <Ryme/Config.hpp>
#include <Ryme/Span.hpp>
#include <Ryme/Math.hpp>

#include <Ryme/ThirdParty/vulkan.hpp>

namespace ryme {

struct RYME_API Vertex
{
    struct AttributeLocation
    {
        static inline uint32_t Position  = 0;
        static inline uint32_t Normal    = 1;
        static inline uint32_t Tangent   = 2;
        static inline uint32_t Color     = 3;
        static inline uint32_t TexCoord  = 4;
        static inline uint32_t Joints    = 5;
        static inline uint32_t Weights   = 6;
    };

    alignas(16) Vec4 Position;

    alignas(16) Vec4 Normal;

    alignas(16) Vec4 Tangent;
    
    alignas(16) Vec4 Color;

    alignas(8) Vec2 TexCoord;

    alignas(8) Vec2u Joints;

    alignas(16) Vec4 Weights;
    
}; // struct Vertex

static_assert(
    sizeof(Vertex) == 96,
    "sizeof(Vertex) does not match GLSL layout std140"
);

Span<vk::VertexInputBindingDescription> GetVertexInputBindingDescriptionList();

Span<vk::VertexInputAttributeDescription> GetVertexInputAttributeDescriptionList();

} // namespace ryme

#endif // RYME_VERTEX_HPP
