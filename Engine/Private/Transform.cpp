#include <Ryme/Transform.hpp>

namespace ryme {

RYME_API
inline Mat4 Transform::ToMatrix() const
{
    Mat4 matrix = Mat4(1.0f);
    matrix = glm::translate(matrix, Position);
    matrix *= glm::mat4_cast(Orientation);
    matrix = glm::scale(matrix, Scale);
    return matrix;
}

} // namespace ryme