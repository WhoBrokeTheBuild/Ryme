#ifndef RYME_PRIMITIVE_HPP
#define RYME_PRIMITIVE_HPP

#include <Ryme/Config.hpp>
#include <Ryme/Containers.hpp>
#include <Ryme/Vertex.hpp>

#include <Ryme/ThirdParty/vulkan.hpp>

namespace ryme {

class RYME_API Primitive
{
public:

    vk::PrimitiveTopology Topology = vk::PrimitiveTopology::eTriangleList;

    List<uint32_t> IndexList;

    List<Vertex> VertexList;

    // Material

    void CalculateTangents();

}; // class Primitive
    
} // namespace ryme

#endif // RYME_PRIMITIVE_HPP