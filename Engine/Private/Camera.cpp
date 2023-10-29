#include <Ryme/Camera.hpp>

namespace ryme {

RYME_API
Mat4 Camera::GetView() const
{
    const Vec3& position = GetWorldPosition();
    return glm::lookAt(position, position + GetForward(), GetUp());
}

RYME_API
Mat4 Camera::GetProjection() const
{
    if (_mode == Mode::Perspective) {
        return glm::perspective(_fovX, GetAspect(), GetNear(), GetFar());
    }
    else if (_mode == Mode::Orthographic) {
        const Vec4& view = GetViewport();
        return glm::ortho(view[0], view[1], view[2], view[3], GetNear(), GetFar());
    }

    return Mat4(1.0f);
}

RYME_API
void Camera::SetForward(const Vec3& forward)
{
    Vec3 check = glm::normalize(forward) + GetUp();
    if (check == Vec3(0.0f)) {
        Transform.Orientation = glm::angleAxis(DegToRad(-90.0f), Vec3(1.0f, 0.0f, 0.0f));
    }
    else {
        Transform.Orientation = glm::quatLookAt(glm::normalize(forward), _up);
    }
}

RYME_API
Vec3 Camera::GetForward() const
{
    return glm::rotate(GetWorldOrientation(), GetWorldForward());
}

RYME_API
Vec3 Camera::GetRight() const
{
    return glm::normalize(glm::cross(GetForward(), GetUp()));
}

RYME_API
void Camera::SetFOVY(float fovY)
{
    _fovX = 2.0f * atanf(tanf(fovY * .5f) * _aspect);
}

RYME_API
void Camera::LookAt(const Vec3& point)
{
    SetForward(point - Transform.Position);
}

RYME_API
Vec4 Camera::GetViewport() const
{
    const Vec4& scale = GetViewportScale();
    Vec2 size = GetViewportSize();

    if (_aspect > 1.0f) {
        size.y /= _aspect;
    }
    else {
        size.x *= _aspect;
    }

    return {
        size.x * scale[0],
        size.x * scale[1],
        size.y * scale[2],
        size.y * scale[3],
    };
}

} // namespace ryme