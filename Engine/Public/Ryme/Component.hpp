#ifndef RYME_COMPONENT_HPP
#define RYME_COMPONENT_HPP

#include <Ryme/Config.hpp>

namespace ryme {

class Entity;

class RYME_API Component
{
public:

    RYME_DISALLOW_COPY_AND_ASSIGN(Component)

    Component() = default;

    virtual ~Component();

    void Attach(Entity * entity);

    void Detach();

    Entity * GetEntity() const {
        return _entity;
    }

    void Update();

    void Render();

protected:

    virtual void AfterAttach() { }

    virtual void BeforeDetach() { }

    virtual void OnUpdate() { }

    virtual void OnRender() { }

private:

    Entity * _entity = nullptr;

}; // class Component

} // namespace ryme

#endif // RYME_COMPONENT_HPP