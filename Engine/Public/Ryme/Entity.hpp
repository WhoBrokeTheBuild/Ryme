#ifndef RYME_ENTITY_HPP
#define RYME_ENTITY_HPP

#include <Ryme/Config.hpp>
#include <Ryme/Component.hpp>
#include <Ryme/Containers.hpp>
#include <Ryme/String.hpp>
#include <Ryme/Transform.hpp>

namespace ryme {

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
    
    void AddComponent(Component * component);

    void RemoveComponent(Component * component);

    inline Span<Component *> GetComponentList() {
        return _componentList;
    }

    void AddChild(Entity * child);

    void RemoveChild(Entity * child);

    inline Span<Entity *> GetChildList() {
        return _childList;
    }
    
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

    List<Entity *> _childList;

    String _name;

    Transform _transform;

}; // class Entity

} // namespace ryme

#endif // RYME_ENTITY_HPP