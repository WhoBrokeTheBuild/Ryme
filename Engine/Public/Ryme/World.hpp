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
System * AddSystem(System * system, TypeIndex typeIndex);

RYME_API
void RemoveSystem(System * system);

RYME_API
Span<System *> GetSystemList();

RYME_API
System * GetSystem(TypeIndex typeIndex);

// Root

RYME_API
Entity * GetRootEntity();

RYME_API
inline Entity * AddEntity(Entity * entity, TypeIndex typeIndex) {
    return GetRootEntity()->AddEntity(entity, typeIndex);
}

RYME_API
inline void RemoveEntity(Entity * entity) {
    return GetRootEntity()->RemoveEntity(entity);
}

RYME_API
inline Span<Entity *> GetEntityList() {
    return GetRootEntity()->GetEntityList();
}

RYME_API
inline Span<Entity *> GetEntityList(TypeIndex typeIndex) {
    return GetRootEntity()->GetEntityList(typeIndex);
}

// Global

RYME_API
Entity * RegisterGlobalEntity(Entity * entity, TypeIndex typeIndex);

RYME_API
void UnregisterGlobalEntity(Entity * entity);

RYME_API
Span<Entity *> GetGlobalEntityList();

RYME_API
Span<Entity *> GetGlobalEntityList(TypeIndex typeIndex);

RYME_API
Component * RegisterGlobalComponent(Component * component, TypeIndex typeIndex);

RYME_API
void UnregisterGlobalComponent(Component * component);

RYME_API
Span<Component *> GetGlobalComponentList();

RYME_API
Span<Component *> GetGlobalComponentList(TypeIndex typeIndex);

// TypeIndex helpers

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

template <typename T>
inline Entity * AddEntity(T * entity) {
    return AddEntity(entity, TypeIndex(typeid(T)));
}

template <typename T>
inline void RemoveEntity(T * entity) {
    return RemoveEntity(entity, TypeIndex(typeid(T)));
}

template <typename T>
inline Span<Entity *> GetEntityList() {
    return GetEntityList(TypeIndex(typeid(T)));
}

template <typename T>
inline void RegisterGlobalEntity(T * entity) {
    RegisterGlobalEntity(entity, TypeIndex(typeid(T)));
}

template <typename T>
inline Span<Entity *> GetGlobalEntityList() {
    return GetGlobalEntityList(TypeIndex(typeid(T)));
}

template <typename T>
inline void RegisterGlobalComponent(T * component) {
    RegisterGlobalComponent(component, TypeIndex(typeid(T)));
}

template <typename T>
inline Span<Component *> GetGlobalComponentList() {
    return GetGlobalComponentList(TypeIndex(typeid(T)));
}

} // namespace World

} // namespace ryme

#endif // RYME_WORLD_HPP