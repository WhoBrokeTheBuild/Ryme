#include <Ryme/Mesh.hpp>

namespace ryme {

RYME_API
Mesh::Mesh(MeshData&& data)
    : _count(data.VertexList.size())
    , _primitiveTopology(data.PrimitiveTopology)
{
    _vertexBuffer.Create(
        data.VertexList.size() * sizeof(Vertex),
        reinterpret_cast<uint8_t *>(data.VertexList.data()),
        vk::BufferUsageFlagBits::eVertexBuffer,
        VMA_MEMORY_USAGE_GPU_ONLY
    );

    if (not data.IndexList.empty()) {
        _indexed = true;
        _count = data.IndexList.size();

        _indexBuffer.Create(
            data.IndexList.size() * sizeof(uint32_t),
            reinterpret_cast<uint8_t *>(data.IndexList.data()),
            vk::BufferUsageFlagBits::eIndexBuffer,
            VMA_MEMORY_USAGE_GPU_ONLY
        );
    }
}

RYME_API
void Mesh::GenerateCommands(vk::CommandBuffer buffer)
{
    if (_indexed) {
        buffer.bindIndexBuffer(_indexBuffer.GetVkBuffer(), 0, vk::IndexType::eUint32);
    }

    buffer.setPrimitiveTopology(_primitiveTopology);

    // TODO: Improve?
    vk::Buffer buffers[] = { _vertexBuffer.GetVkBuffer() };
    vk::DeviceSize offsets[] = { 0 };
    buffer.bindVertexBuffers(0, buffers, offsets);

    if (_indexed) {
        buffer.drawIndexed(_count, 1, 0, 0, 0);
    }
    else {
        buffer.draw(_count, 1, 0, 0);
    }
}

RYME_API
void MeshData::CalculateTangents()
{
    auto processTriangle = [](Vertex& v1, Vertex& v2, Vertex& v3)
    {
        Vec3 v = Vec3(v2.Position) - Vec3(v1.Position);
        Vec3 w = Vec3(v3.Position) - Vec3(v1.Position);
        Vec2 s = v2.TexCoord - v1.TexCoord;
        Vec2 t = v3.TexCoord - v1.TexCoord;

        float dir = 1.0f;
        if ((t.x * s.y - t.y * s.x) < 0.0f) {
            dir = -1.0f;
        }

        Vec3 tangent = {
            dir * (w.x * s.y - v.x * t.y),
            dir * (w.y * s.y - v.y * t.y),
            dir * (w.z * s.y - v.z * t.y),
        };

        v1.Tangent = glm::normalize(Vec4(tangent - Vec3(v1.Normal) * (tangent * Vec3(v1.Normal)), 1.0f));
        v2.Tangent = glm::normalize(Vec4(tangent - Vec3(v2.Normal) * (tangent * Vec3(v2.Normal)), 1.0f));
        v3.Tangent = glm::normalize(Vec4(tangent - Vec3(v3.Normal) * (tangent * Vec3(v3.Normal)), 1.0f));
        
        // TODO: Account for NaN and smoothing
    };

    // Computing tangents for other topologies can cause issues with averaging and such
    // best to just use TriangleList, or let the modeling software generate the tangents
    if (PrimitiveTopology == vk::PrimitiveTopology::eTriangleList) {
        if (IndexList.empty()) {
            for (size_t i = 0; i < VertexList.size(); i += 3) {
                processTriangle(
                    VertexList[i + 0],
                    VertexList[i + 1],
                    VertexList[i + 2]
                );
            }
        }
        else {
            for (size_t i = 0; i < IndexList.size(); i += 3) {
                processTriangle(
                    VertexList[IndexList[i + 0]],
                    VertexList[IndexList[i + 1]],
                    VertexList[IndexList[i + 2]]
                );
            }
        }
    }
}

} // namespace ryme