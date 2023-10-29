#ifndef RYME_MESH_HPP
#define RYME_MESH_HPP

#include <Ryme/Config.hpp>
#include <Ryme/Asset.hpp>
#include <Ryme/Buffer.hpp>
#include <Ryme/List.hpp>
#include <Ryme/NonCopyable.hpp>
#include <Ryme/Vertex.hpp>

#include <Ryme/ThirdParty/vulkan.hpp>

namespace ryme {

class RYME_API MeshData
{
public:

    vk::PrimitiveTopology PrimitiveTopology = vk::PrimitiveTopology::eTriangleList;

    List<uint32_t> IndexList;

    List<Vertex> VertexList;

    // Material

    void CalculateTangents();

}; // class MeshData

class RYME_API Mesh : public NonCopyable
{
public:

    Mesh(MeshData&& data);
    
    Mesh(Mesh&&) = default;

    virtual ~Mesh() = default;

    void GenerateCommands(vk::CommandBuffer buffer);

private:

    bool _indexed = false;

    uint32_t _count;

    vk::PrimitiveTopology _primitiveTopology;

    Buffer _vertexBuffer;

    Buffer _indexBuffer;

}; // class Mesh

} // namespace ryme

#endif // RYME_MESH_HPP