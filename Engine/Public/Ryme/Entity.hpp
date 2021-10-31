#ifndef RYME_ENTITY_HPP
#define RYME_ENTITY_HPP

#include <Ryme/Config.hpp>
#include <Ryme/Component.hpp>
#include <Ryme/Containers.hpp>
#include <Ryme/String.hpp>
#include <Ryme/Transform.hpp>
#include <Ryme/Types.hpp>

namespace ryme {

using TypeIndex = TypeIndex;

class RYME_API Entity
{
public:

    RYME_DISALLOW_COPY_AND_ASSIGN(Entity)

    Entity() = default;

    virtual ~Entity();

    Entity * GetParent() const {
        return _parent;
    }

    inline String GetName() const {
        return _name;
    }

    void SetName(String name);

    Component * AddComponent(Component * component, TypeIndex typeIndex);
    
    void RemoveComponent(Component * component);

    inline Span<Component *> GetComponentList() {
        return _componentList;
    }

    Span<Component *> GetComponentList(TypeIndex typeIndex);

    Entity * AddEntity(Entity * entity, TypeIndex typeIndex);

    void RemoveEntity(Entity * entity);

    inline Span<Entity *> GetEntityList() {
        return _entityList;
    }

    Span<Entity *> GetEntityList(TypeIndex typeIndex);
    
    inline Transform& GetTransform() {
        return _transform;
    }

    inline Transform GetTransform() const {
        return _transform;
    }

    void SetTransform(Transform transform);

    Transform GetWorldTransform() const;

    inline Vec3 GetPosition() const {
        return _transform.Position;
    }

    void SetPosition(Vec3 position);

    Vec3 GetWorldPosition() const;

    inline Quat GetOrientation() const {
        return _transform.Orientation;
    }

    void SetOrientation(Quat orientation);

    Quat GetWorldOrientation() const;

    inline Vec3 GetScale() const {
        return _transform.Scale;
    }

    void SetScale(Vec3 scale);

    Quat GetWorldScale() const;

    void Update();

    void Render();

    // TypeIndex helpers
    
    template <typename T>
    inline Component * AddComponent(T * component) {
        return AddComponent(component, TypeIndex(typeid(T)));
    }

    template <typename T>
    inline Span<Component *> GetComponentList() {
        return GetComponentList(TypeIndex(typeid(T)));
    }

    template <typename T>
    inline Entity * AddEntity(T * entity) {
        return AddEntity(entity, TypeIndex(typeid(T)));
    }

    template <typename T>
    inline Span<Entity *> GetEntityList() {
        return GetEntityList(TypeIndex(typeid(T)));
    }

protected:

    virtual void BeforeUpdate() { };

    virtual void OnUpdate() { };

    virtual void AfterUpdate() { };

    virtual void BeforeRender() { };

    virtual void OnRender() { };

    virtual void AfterRender() { };

private:

    Entity * _parent = nullptr;

    List<Component *> _componentList;

    Map<TypeIndex, List<Component *>> _componentListTypeMap;

    List<Entity *> _entityList;

    Map<TypeIndex, List<Entity *>> _entityListTypeMap;

    String _name;

    Transform _transform;

}; // class Entity

} // namespace ryme

#endif // RYME_ENTITY_HPP