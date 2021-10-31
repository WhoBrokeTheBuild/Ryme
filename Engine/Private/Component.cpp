#include <Ryme/Component.hpp>
#include <Ryme/Entity.hpp>

namespace ryme {

Component::~Component()
{
    if (_entity) {
        _entity->RemoveComponent(this);
    }
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