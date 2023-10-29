#ifndef RYME_CAMERA_HPP
#define RYME_CAMERA_HPP

#include <Ryme/Config.hpp>
#include <Ryme/Math.hpp>
#include <Ryme/Entity.hpp>

namespace ryme {

class RYME_API Camera : public Entity
{
public:
    
    enum class Mode
    {
        Perspective,
        Orthographic,
    };

    Camera() = default;
    
    virtual ~Camera() = default;

    Mat4 GetView() const;

    Mat4 GetProjection() const;

    inline void SetMode(Mode mode) {
        _mode = mode;
    }

    inline Mode GetMode() const {
        return _mode;
    }

    inline void SetClip(const Vec2& clip) {
        _clip = clip;
    }

    inline Vec2 GetClip() const {
        return _clip;
    }

    inline float GetNear() const {
        return _clip[0];
    }

    inline float GetFar() const {
        return _clip[1];
    }

    inline void SetAspect(float aspect) {
        _aspect = aspect;
    }

    inline void SetAspect(const Vec2& size) {
        _aspect = size.x / size.y;
    }

    inline float GetAspect() const {
        return _aspect;
    }

    inline void SetUp(const Vec3& up) {
        _up = up;
    }

    inline Vec3 GetUp() const {
        return _up;
    }

    void SetForward(const Vec3& forward);

    Vec3 GetForward() const;

    Vec3 GetRight() const;

    // Perspective

    inline void SetFOVX(float fovX) {
        _fovX = fovX;
    }

    inline float GetFOVX() const {
        return _fovX;
    }

    void SetFOVY(float fovY);

    // float GetFOVY() const;

    void LookAt(const Vec3& point);

    // Orthographic

    inline void SetViewportSize(const Vec2& size) {
        _viewportSize = size;
    }

    inline Vec2 GetViewportSize() const {
        return _viewportSize;
    }

    inline void SetViewportScale(const Vec4& scale) {
        _viewportScale = scale;
    }

    inline Vec4 GetViewportScale() const {
        return _viewportScale;
    }

    Vec4 GetViewport() const;


    // constexpr const char * ModeToString(Mode mode)
    // {
    //     switch (mode)
    //     {
    //     case Mode::Perspective:
    //         return "Perspective";
    //     case Mode::Orthographic:
    //         return "Orthographic";
    //     }
    // }

private:

    Mode _mode = Mode::Perspective;

    Vec2 _clip = { 0.1f, 10000.0f };

    float _aspect = 16.0f / 9.0f; // 16:9

    Vec3 _up = GetWorldUp();

    // Perspective

    float _fovX = DegToRad(45.0f);

    // Orthographic

    Vec2 _viewportSize = { 1920.0f, 1080.0f };

    Vec4 _viewportScale = { -0.5f, 0.5f, 0.5f, -0.5f };

}; // class Camera

} // namespace ryme

#endif // RYME_CAMERA_HPP