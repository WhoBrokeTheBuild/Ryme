#ifndef RYME_ENTITY_HPP
#define RYME_ENTITY_HPP

#include <Ryme/Config.hpp>
#include <Ryme/Component.hpp>
#include <Ryme/List.hpp>
#include <Ryme/Math.hpp>
#include <Ryme/NonCopyable.hpp>
#include <Ryme/Span.hpp>
#include <Ryme/String.hpp>
#include <Ryme/Transform.hpp>

namespace ryme {

RYME_API
constexpr Vec3 GetWorldUp() {
    return { 0.0f, 1.0f, 0.0f };
}

RYME_API 
constexpr Vec3 GetWorldForward() {
    return { 0.0f, 0.0f, -1.0f };
}

class Scene;

class RYME_API Entity : public NonCopyable
{
public:

    Transform Transform;

    Entity() = default;

    Entity(StringView name);
    
    virtual ~Entity();

    inline void SetParent(Entity * parent) {
        _parent = parent;
    }

    inline Entity * GetParent() const {
        return _parent;
    }

    Scene * GetScene();

    template <class T>
    T * AddChild(T * entity)
    {
        static_assert(std::is_base_of<Entity, T>::value);

        Entity * baseEntity = static_cast<Entity *>(entity);
        baseEntity->SetParent(this);
        _childList.push_back(baseEntity);

        return entity;
    }

    Span<Entity *> GetChildList() {
        return { _childList.begin(), _childList.end() };
    }

    template <class T>
    T * AddComponent(T * component)
    {
        static_assert(std::is_base_of<Component, T>::value);
        
        Component * baseComponent = static_cast<Component *>(component);
        _componentList.push_back(baseComponent);
        baseComponent->Attach(this);
        // _componentMapByType.emplace(typeid(T), baseComponent);

        return component;
    }

    // template <class T>
    // T * GetComponent()
    // {
    //     auto it = _componentMapByType.find(typeid(T));
    //     if (it != _componentMapByType.end()) {
    //         return static_cast<T *>(it->second);
    //     }

    //     return nullptr;
    // }

    Span<Component *> GetComponentList() const;

    inline void SetName(StringView name) {
        _name = name;
    }

    inline StringView GetName() const {
        return _name;
    }

    inline Vec3 GetWorldPosition() const {
        if (_parent) {
            return _parent->GetWorldPosition() + Transform.Position;
        }

        return Transform.Position;
    }

    inline Quat GetWorldOrientation() const {
        if (_parent) {
            return _parent->GetWorldOrientation() * Transform.Orientation;
        }

        return Transform.Orientation;
    }

    inline Vec3 GetWorldScale() const {
        if (_parent) {
            return _parent->GetWorldScale() * Transform.Scale;
        }

        return Transform.Scale;
    }

    inline ryme::Transform GetWorldTransform() const {
        // TODO: Replace with while loop?
        if (_parent) {
            return _parent->GetWorldTransform() + Transform;
        }

        return Transform;
    }

private:

    Entity * _parent = nullptr;

    String _name;

    List<Entity *> _childList;

    List<Component *> _componentList;

    // Map<TypeIndex, Component *> _componentMapByType;

}; // class Entity

} // namespace ryme

#endif // RYME_ENTITY_HPP