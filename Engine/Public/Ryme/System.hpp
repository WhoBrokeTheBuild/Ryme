#ifndef RYME_SYSTEM_HPP
#define RYME_SYSTEM_HPP

#include <Ryme/Config.hpp>
#include <Ryme/Component.hpp>
#include <Ryme/Entity.hpp>
#include <Ryme/Types.hpp>

namespace ryme {

class RYME_API System
{
public:

    RYME_DISALLOW_COPY_AND_ASSIGN(System)

    System() = default;

    virtual ~System() = default;

    void Update();

    void Render();

    virtual void OnEntityRegistered(Entity * entity, TypeIndex typeIndex) { }

    virtual void OnEntityUnregistered(Entity * entity) { }

    virtual void OnComponentRegistered(Component * component, TypeIndex typeIndex) { }

    virtual void OnComponentUnregistered(Component * component) { }

protected:

    virtual void OnUpdate() { }

    virtual void OnRender() { }

private:

}; // class System

} // namespace ryme

#endif // RYME_SYSTEM_HPP