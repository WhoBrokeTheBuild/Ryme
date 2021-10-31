#include <Ryme/Entity.hpp>
#include <Ryme/World.hpp>

namespace ryme {

Entity::~Entity()
{
    if (_parent) {
        _parent->RemoveEntity(this);
    }

    for (auto component : _componentList) {
        component->Detach();
        delete component;
    }

    for (auto entity : _entityList) {
        entity->_parent = nullptr;
        delete entity;
    }

    World::UnregisterGlobalEntity(this);
}

void Entity::SetName(String name)
{
    _name = name;
}

Component * Entity::AddComponent(Component * component, TypeIndex typeIndex)
{
    World::RegisterGlobalComponent(component, typeIndex);

    auto it = std::find(_componentList.begin(), _componentList.end(), component);
    if (it == _componentList.end()) {
        _componentList.push_back(component);

        if (!_componentListTypeMap.contains(typeIndex)) {
            _componentListTypeMap.emplace(typeIndex, List<Component *>());
        }

        _componentListTypeMap[typeIndex].push_back(component);
    }
    
    component->Attach(this);

    return component;
}

void Entity::RemoveComponent(Component * component)
{
    World::UnregisterGlobalComponent(component);

    auto it = std::find(_componentList.begin(), _componentList.end(), component);
    if (it != _componentList.end()) {
        _componentList.erase(it);
    }

    for (auto& [key, list] : _componentListTypeMap) {
        it = std::find(list.begin(), list.end(), component);
        if (it != list.end()) {
            list.erase(it);
            break;
        }
    }

    component->Detach();
}

Span<Component *> Entity::GetComponentList(TypeIndex typeIndex)
{
    auto it = _componentListTypeMap.find(typeIndex);
    if (it != _componentListTypeMap.end()) {
        return it->second;
    }

    return Span<Component *>();
}

Entity * Entity::AddEntity(Entity * entity, TypeIndex typeIndex)
{
    World::RegisterGlobalEntity(entity, typeIndex);

    auto it = std::find(_entityList.begin(), _entityList.end(), entity);
    if (it == _entityList.end()) {
        _entityList.push_back(entity);
        
        if (!_entityListTypeMap.contains(typeIndex)) {
            _entityListTypeMap.emplace(typeIndex, List<Entity *>());
        }

        _entityListTypeMap[typeIndex].push_back(entity);
    }

    if (entity->_parent) {
        entity->_parent->RemoveEntity(entity);
    }
    
    entity->_parent = this;

    return entity;
}

void Entity::RemoveEntity(Entity * entity)
{
    World::UnregisterGlobalEntity(entity);

    auto it = std::find(_entityList.begin(), _entityList.end(), entity);
    if (it != _entityList.end()) {
        _entityList.erase(it);
    }

    for (auto& [key, list] : _entityListTypeMap) {
        it = std::find(list.begin(), list.end(), entity);
        if (it != list.end()) {
            list.erase(it);
            break;
        }
    }

    entity->_parent = nullptr;
}

Span<Entity *> Entity::GetEntityList(TypeIndex typeIndex)
{
    auto it = _entityListTypeMap.find(typeIndex);
    if (it != _entityListTypeMap.end()) {
        return it->second;
    }

    return Span<Entity *>();
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

    for (auto entity : _entityList) {
        entity->Update();
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

    for (auto entity : _entityList) {
        entity->Render();
    }

    AfterRender();
}

} // namespace ryme