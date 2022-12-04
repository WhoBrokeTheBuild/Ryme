#include <Ryme/Primitive.hpp>

namespace ryme {

void Primitive::CalculateTangents()
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
    if (Topology == vk::PrimitiveTopology::eTriangleList) {
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
