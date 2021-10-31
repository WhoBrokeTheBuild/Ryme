#include <Ryme/World.hpp>

namespace ryme {

namespace World {

Entity * _root = nullptr;

List<Component *> _componentList;

Map<TypeIndex, List<Component *>> _componentListTypeMap;

List<Entity *> _entityList;

Map<TypeIndex, List<Entity *>> _entityListTypeMap;

List<System *> _systemList;

Map<TypeIndex, System *> _systemTypeMap;

RYME_API
void Init()
{
    Term();
    
    _root = new Entity();
    _root->SetName("ryme::World");
}

RYME_API
void Term()
{
    if (_root) {
        delete _root;
        _root = nullptr;
    }

    Reset();
}

RYME_API
void Reset()
{
    for (auto system : _systemList) {
        delete system;
    }
    
    _systemList.clear();
    _systemTypeMap.clear();

    for (auto entity : _entityList) {
        delete entity;
    }

    _entityList.clear();
    _entityListTypeMap.clear();

    for (auto component : _componentList) {
        delete component;
    }

    _componentList.clear();
    _componentListTypeMap.clear();
}

RYME_API
void Update()
{
    assert(_root);

    for (auto system : _systemList) {
        system->Update();
    }

    _root->Update();
}

RYME_API
void Render()
{
    assert(_root);

    for (auto system : _systemList) {
        system->Render();
    }

    _root->Render();
}

RYME_API
System * AddSystem(System * system, TypeIndex typeIndex)
{
    auto it = std::find(_systemList.begin(), _systemList.end(), system);
    if (it == _systemList.end()) {
        _systemList.push_back(system);
        _systemTypeMap[typeIndex] = system;
    }

    return system;
}

RYME_API
void RemoveSystem(System * system)
{
    auto it = std::find(_systemList.begin(), _systemList.end(), system);
    if (it != _systemList.end()) {
        _systemList.erase(it);
    }

    for (auto& [key, value] : _systemTypeMap) {
        if (value == system) {
            _systemTypeMap.erase(key);
            break;
        }
    }
}

RYME_API
Span<System *> GetSystemList()
{
    return _systemList;
}

RYME_API
System * GetSystem(TypeIndex typeIndex)
{
    auto it = _systemTypeMap.find(typeIndex);
    if (it != _systemTypeMap.end()) {
        return it->second;
    }

    return nullptr;
}

RYME_API
Entity * GetRootEntity()
{
    return _root;
}

RYME_API
Entity * RegisterGlobalEntity(Entity * entity, TypeIndex typeIndex)
{
    auto it = std::find(_entityList.begin(), _entityList.end(), entity);
    if (it == _entityList.end()) {
        _entityList.push_back(entity);
        
        if (!_entityListTypeMap.contains(typeIndex)) {
            _entityListTypeMap.emplace(typeIndex, List<Entity *>());
        }

        _entityListTypeMap[typeIndex].push_back(entity);
    }
    
    for (auto system : _systemList) {
        system->OnEntityRegistered(entity, typeIndex);
    }

    return entity;
}

RYME_API
void UnregisterGlobalEntity(Entity * entity)
{
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
    
    for (auto system : _systemList) {
        system->OnEntityUnregistered(entity);
    }
}

RYME_API
Span<Entity *> GetGlobalEntityList()
{
    return _entityList;
}

RYME_API
Span<Entity *> GetGlobalEntityList(TypeIndex typeIndex)
{
    auto it = _entityListTypeMap.find(typeIndex);
    if (it != _entityListTypeMap.end()) {
        return it->second;
    }

    return Span<Entity *>();
}

RYME_API
Component * RegisterGlobalComponent(Component * component, TypeIndex typeIndex)
{
    auto it = std::find(_componentList.begin(), _componentList.end(), component);
    if (it == _componentList.end()) {
        _componentList.push_back(component);

        if (!_componentListTypeMap.contains(typeIndex)) {
            _componentListTypeMap.emplace(typeIndex, List<Component *>());
        }

        _componentListTypeMap[typeIndex].push_back(component);
    }

    for (auto system : _systemList) {
        system->OnComponentRegistered(component, typeIndex);
    }

    return component;
}

RYME_API
void UnregisterGlobalComponent(Component * component)
{
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

    for (auto system : _systemList) {
        system->OnComponentUnregistered(component);
    }
}

RYME_API
Span<Component *> GetGlobalComponentList()
{
    return _componentList;
}

RYME_API
Span<Component *> GetGlobalComponentList(TypeIndex typeIndex)
{
    auto it = _componentListTypeMap.find(typeIndex);
    if (it != _componentListTypeMap.end()) {
        return it->second;
    }

    return Span<Component *>();
}

} // namespace World

} // namespace ryme