#include <Ryme/ModelComponent.hpp>
#include <Ryme/Scene.hpp>
#include <Ryme/RenderSystem.hpp>

namespace ryme {

ModelComponent::ModelComponent(Model * model)
    : _model(model)
{ }

ModelComponent::~ModelComponent()
{
    delete _model;
}

RYME_API
void ModelComponent::Attach(Entity * entity)
{
    Component::Attach(entity);

    Scene * scene = GetEntity()->GetScene();
    if (scene) {
        RenderSystem * rs = scene->GetSystem<RenderSystem>();
        if (rs) {
            rs->AddModelComponent(this);
        }
    }
}

RYME_API
void ModelComponent::Detach()
{
    Scene * scene = GetEntity()->GetScene();
    if (scene) {
        RenderSystem * rs = scene->GetSystem<RenderSystem>();
        if (rs) {
            rs->RemoveModelComponent(this);
        }
    }


    Component::Detach();
}

} // namespace ryme