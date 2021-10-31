#include <Ryme/World.hpp>

namespace ryme {

namespace World {

Entity * _root = nullptr;

RYME_API
void Init()
{
    if (_root) {
        delete _root;
        _root = nullptr;
    }

    _root = new Entity();
    _root->SetName("Ryme::World");
}

RYME_API
void Term()
{
    delete _root;
    _root = nullptr;
}

RYME_API
Entity * GetRootEntity()
{
    return _root;
}

RYME_API
void AddChild(Entity * child)
{
    assert(_root);
    assert(child);

    _root->AddChild(child);
}

RYME_API
void RemoveChild(Entity * child)
{
    assert(_root);
    assert(child);

    _root->RemoveChild(child);
}

RYME_API
Span<Entity *> GetChildList()
{
    assert(_root);

    return _root->GetChildList();
}

RYME_API
void Update()
{
    assert(_root);

    _root->Update();
}

RYME_API
void Render()
{
    assert(_root);

    _root->Render();
}

} // namespace World

} // namespace ryme