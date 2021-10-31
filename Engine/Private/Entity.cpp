#include <Ryme/Entity.hpp>

namespace ryme {

Entity::~Entity()
{
    if (_parent) {
        _parent->RemoveChild(this);
    }

    for (auto component : _componentList) {
        component->Detach();
        delete component;
    }

    for (auto child : _childList) {
        child->_parent = nullptr;
        delete child;
    }
}

void Entity::SetName(String name)
{
    _name = name;
}

void Entity::AddComponent(Component * component)
{
    auto it = std::find(_componentList.begin(), _componentList.end(), component);
    if (it == _componentList.end()) {
        _componentList.push_back(component);
    }
    
    component->Attach(this);
}

void Entity::RemoveComponent(Component * component)
{
    auto it = std::find(_componentList.begin(), _componentList.end(), component);
    if (it != _componentList.end()) {
        _componentList.erase(it);
    }

    component->Detach();
}

void Entity::AddChild(Entity * child)
{
    auto it = std::find(_childList.begin(), _childList.end(), child);
    if (it == _childList.end()) {
        _childList.push_back(child);
    }

    if (child->_parent) {
        child->_parent->RemoveChild(child);
    }
    
    child->_parent = this;
}

void Entity::RemoveChild(Entity * child)
{
    auto it = std::find(_childList.begin(), _childList.end(), child);
    if (it != _childList.end()) {
        _childList.erase(it);
    }
    
    child->_parent = nullptr;
}

void Entity::SetTransform(Transform transform)
{
    _transform = transform;
}

Transform Entity::GetWorldTransform() const
{
    if (_parent) {
        return _parent->GetWorldTransform() + GetTransform();
    }

    return GetTransform();
}

void Entity::SetPosition(Vec3 position)
{
    _transform.Position = position;
}

Vec3 Entity::GetWorldPosition() const
{
    if (_parent) {
        return _parent->GetWorldPosition() + GetPosition();
    }

    return GetPosition();
}

void Entity::SetOrientation(Quat orientation)
{
    _transform.Orientation = orientation;
}

Quat Entity::GetWorldOrientation() const
{
    if (_parent) {
        return _parent->GetWorldOrientation() * GetOrientation();
    }

    return GetOrientation();
}

void Entity::SetScale(Vec3 scale)
{
    _transform.Scale = scale;
}

Quat Entity::GetWorldScale() const
{
    if (_parent) {
        return _parent->GetWorldScale() * GetScale();
    }

    return GetScale();
}

void Entity::Update()
{
    BeforeUpdate();

    OnUpdate();

    for (auto component : _componentList) {
        component->Update();
    }

    for (auto child : _childList) {
        child->Update();
    }

    AfterUpdate();
}

void Entity::Render()
{
    BeforeRender();

    OnRender();

    for (auto component : _componentList) {
        component->Render();
    }

    for (auto child : _childList) {
        child->Render();
    }

    AfterRender();
}

} // namespace ryme