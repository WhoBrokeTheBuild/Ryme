#ifndef RYME_MODEL_COMPONENT_HPP
#define RYME_MODEL_COMPONENT_HPP

#include <Ryme/Config.hpp>
#include <Ryme/Component.hpp>
#include <Ryme/Model.hpp>

namespace ryme {

class RYME_API ModelComponent : public Component
{
public:

    ModelComponent(Model * model);

    virtual ~ModelComponent();

    void Attach(Entity * entity) override;

    void Detach() override;

    Model * GetModel() const {
        return _model;
    }

private:

    Model * _model;

}; // class ModelComponent

} // namespace ryme

#endif // RYME_MODEL_COMPONENT_HPP