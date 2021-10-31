#ifndef RYME_WORLD_HPP
#define RYME_WORLD_HPP

#include <Ryme/Config.hpp>
#include <Ryme/Component.hpp>
#include <Ryme/Entity.hpp>
#include <Ryme/System.hpp>
#include <Ryme/Types.hpp>

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
void Update();

RYME_API
void Render();

RYME_API
Entity * GetRootEntity();

RYME_API
Entity * RegisterEntity(Entity * entity, TypeIndex typeIndex);

RYME_API
void UnregisterEntity(Entity * entity);

RYME_API
Span<Entity *> GetEntityList();

RYME_API
Span<Entity *> GetEntityList(TypeIndex typeIndex);

RYME_API
Component * RegisterComponent(Component * component, TypeIndex typeIndex);

RYME_API
void UnregisterComponent(Component * component);

RYME_API
Span<Component *> GetComponentList();

RYME_API
Span<Component *> GetComponentList(TypeIndex typeIndex);

RYME_API
System * AddSystem(System * system, TypeIndex typeIndex);

RYME_API
void RemoveSystem(System * system);

RYME_API
Span<System *> GetSystemList();

RYME_API
System * GetSystem(TypeIndex typeIndex);

// TypeIndex helpers

template <typename T>
inline void RegisterComponent(T * component) {
    RegisterComponent(component, TypeIndex(typeid(T)));
}

template <typename T>
inline Span<Component *> GetComponentList() {
    return GetComponentList(TypeIndex(typeid(T)));
}

template <typename T>
inline void RegisterEntity(T * entity) {
    RegisterEntity(entity, TypeIndex(typeid(T)));
}

template <typename T>
inline Span<Entity *> GetEntityList() {
    return GetEntityList(TypeIndex(typeid(T)));
}

template <typename T>
inline void AddSystem(T * system) {
    AddSystem(system, TypeIndex(typeid(T)));
}

template <typename T>
inline Span<System *> GetSystemList() {
    return GetSystemList(TypeIndex(typeid(T)));
}

template <typename T>
inline Span<System *> GetSystem() {
    return GetSystemList(TypeIndex(typeid(T)));
}

} // namespace World

} // namespace ryme

#endif // RYME_WORLD_HPP