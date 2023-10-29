#include <Ryme/RenderSystem.hpp>
#include <Ryme/Log.hpp>

namespace ryme {

void RenderSystem::AddModelComponent(ModelComponent * modelComponent)
{
    Log(RYME_ANCHOR, "Adding model");

    _modelComponentList.push_back(modelComponent);
}

void RenderSystem::RemoveModelComponent(ModelComponent * modelComponent)
{
    Log(RYME_ANCHOR, "Removing model");

    ListRemove(_modelComponentList, modelComponent);
}

} // namespace ryme