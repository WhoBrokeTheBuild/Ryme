#include <Ryme/Vertex.hpp>

namespace ryme {
    
Array<vk::VertexInputBindingDescription, 1> _vertexInputBindingDescriptionList = {
    vk::VertexInputBindingDescription()
        .setBinding(0)
        .setStride(sizeof(Vertex))
        .setInputRate(vk::VertexInputRate::eVertex)
};

Array<vk::VertexInputAttributeDescription, 8> _vertexInputAttributeDescriptionList = {
    vk::VertexInputAttributeDescription()
        .setLocation(Vertex::AttributeLocation::Position)
        .setBinding(0)
        .setFormat(vk::Format::eR32G32B32A32Sfloat)
        .setOffset(offsetof(Vertex, Position)),

    vk::VertexInputAttributeDescription()
        .setLocation(Vertex::AttributeLocation::Normal)
        .setBinding(0)
        .setFormat(vk::Format::eR32G32B32A32Sfloat)
        .setOffset(offsetof(Vertex, Normal)),

    vk::VertexInputAttributeDescription()
        .setLocation(Vertex::AttributeLocation::Tangent)
        .setBinding(0)
        .setFormat(vk::Format::eR32G32B32A32Sfloat)
        .setOffset(offsetof(Vertex, Tangent)),

    vk::VertexInputAttributeDescription()
        .setLocation(Vertex::AttributeLocation::Color)
        .setBinding(0)
        .setFormat(vk::Format::eR32G32B32A32Sfloat)
        .setOffset(offsetof(Vertex, Color)),

    vk::VertexInputAttributeDescription()
        .setLocation(Vertex::AttributeLocation::TexCoord1)
        .setBinding(0)
        .setFormat(vk::Format::eR32G32Sfloat)
        .setOffset(offsetof(Vertex, TexCoord1)),

    vk::VertexInputAttributeDescription()
        .setLocation(Vertex::AttributeLocation::TexCoord2)
        .setBinding(0)
        .setFormat(vk::Format::eR32G32Sfloat)
        .setOffset(offsetof(Vertex, TexCoord2)),

    vk::VertexInputAttributeDescription()
        .setLocation(Vertex::AttributeLocation::Joints)
        .setBinding(0)
        .setFormat(vk::Format::eR32G32B32A32Uint)
        .setOffset(offsetof(Vertex, Joints)),

    vk::VertexInputAttributeDescription()
        .setLocation(Vertex::AttributeLocation::Weights)
        .setBinding(0)
        .setFormat(vk::Format::eR32G32B32A32Sfloat)
        .setOffset(offsetof(Vertex, Weights)),
};

Span<vk::VertexInputBindingDescription> GetVertexInputBindingDescriptionList()
{
    return _vertexInputBindingDescriptionList;
}

Span<vk::VertexInputAttributeDescription> GetVertexInputAttributeDescriptionList()
{
    return _vertexInputAttributeDescriptionList;
}

} // namespace ryme