#include <Ryme/Component.hpp>
#include <Ryme/Entity.hpp>
#include <Ryme/World.hpp>

namespace ryme {

Component::~Component()
{
    if (_entity) {
        _entity->RemoveComponent(this);
    }
    
    World::UnregisterComponent(this);
}

void Component::Attach(Entity * entity)
{
    _entity = entity;

    AfterAttach();
}

void Component::Detach()
{
    BeforeDetach();
    
    _entity = nullptr;
}

void Component::Update()
{
    OnUpdate();
}

void Component::Render()
{
    OnRender();
}

} // namespace ryme