#ifndef RYME_COMPONENT_HPP
#define RYME_COMPONENT_HPP

#include <Ryme/Config.hpp>
#include <Ryme/NonCopyable.hpp>

namespace ryme {

class Entity;

class RYME_API Component : public NonCopyable
{
public:

    Component() = default;

    virtual ~Component() = default;

    virtual inline void Attach(Entity * entity) {
        _entity = entity;
    }

    virtual inline void Detach() {
        _entity = nullptr;
    }

    virtual inline bool IsAttached() const {
        return (_entity != nullptr);
    }

    virtual Entity * GetEntity() const {
        return _entity;
    }

private:

    Entity * _entity = nullptr;

}; // class Component

} // namespace ryme

#endif // RYME_COMPONENT_HPP