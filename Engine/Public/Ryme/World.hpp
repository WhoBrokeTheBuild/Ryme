#ifndef RYME_WORLD_HPP
#define RYME_WORLD_HPP

#include <Ryme/Config.hpp>
#include <Ryme/Entity.hpp>

namespace ryme {

namespace World {

RYME_API
constexpr inline Vec3 GetUp() {
    return { 0.0f, 1.0f, 0.0f };
}

RYME_API
constexpr inline Vec3 GetForward() {
    return { 0.0f, 0.0f, -1.0f };
}

RYME_API
void Init();

RYME_API
void Term();

RYME_API
void Reset();

RYME_API
Entity * GetRootEntity();

RYME_API
void AddChild(Entity * child);

RYME_API
void RemoveChild(Entity * child);

RYME_API
Span<Entity *> GetChildList();

RYME_API
void Update();

RYME_API
void Render();

} // namespace World

} // namespace ryme

#endif // RYME_WORLD_HPP