#ifndef RYME_RENDER_SYSTEM_HPP
#define RYME_RENDER_SYSTEM_HPP

#include <Ryme/Config.hpp>
#include <Ryme/System.hpp>
#include <Ryme/ModelComponent.hpp>

namespace ryme {

class RYME_API RenderSystem : public System
{
public:

    RenderSystem() = default;

    virtual ~RenderSystem() = default;

    void AddModelComponent(ModelComponent * modelComponent);

    void RemoveModelComponent(ModelComponent * modelComponent);

private:

    List<ModelComponent *> _modelComponentList;

}; // class RenderSystem

} // namespace ryme

#endif // RYME_RENDER_SYSTEM_HPP