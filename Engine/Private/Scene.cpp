#include <Ryme/Scene.hpp>
#include <Ryme/Log.hpp>

namespace ryme {

static Scene * _currentScene = nullptr;

Scene::~Scene()
{
    Log(RYME_ANCHOR, "Scene::~Scene");

    // TODO: Clear entities before systems?

    for (System * system : _systemList) {
        delete system;
    }
}

RYME_API
void SetCurrentScene(Scene * scene)
{
    _currentScene = scene;
}

RYME_API
Scene * GetCurrentScene()
{
    return _currentScene;
}

} // namespace ryme