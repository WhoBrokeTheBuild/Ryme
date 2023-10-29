#include <Ryme/Entity.hpp>
#include <Ryme/Scene.hpp>
#include <Ryme/Log.hpp>

namespace ryme {

RYME_API
Entity::Entity(StringView name)
    : _name(name)
{ }

RYME_API
Entity::~Entity()
{
    Log(RYME_ANCHOR, "Entity::~Entity");

    for (auto component : _componentList) {
        component->Detach();
        delete component;
    }

    for (auto child : _childList) {
        delete child;
    }
}

RYME_API
Scene * Entity::GetScene()
{
    if (GetParent() == nullptr) {
        return dynamic_cast<Scene *>(this);
    }

    return GetParent()->GetScene();
}


} // namespace ryme