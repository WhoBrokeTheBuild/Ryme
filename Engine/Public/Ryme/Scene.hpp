#ifndef RYME_SCENE_HPP
#define RYME_SCENE_HPP

#include <Ryme/Config.hpp>
#include <Ryme/Entity.hpp>
#include <Ryme/List.hpp>
#include <Ryme/Map.hpp>
#include <Ryme/System.hpp>
#include <Ryme/Types.hpp>

namespace ryme {

class RYME_API Scene : public Entity
{
public:

    Scene() = default;

    virtual ~Scene();

    template <class T>
    T * AddSystem(T * system)
    {
        static_assert(std::is_base_of<System, T>::value);
        
        System * baseSystem = static_cast<System *>(system);
        _systemList.push_back(baseSystem);
        _systemMapByType.emplace(typeid(T), baseSystem);

        return system;
    }

    template <class T>
    T * GetSystem()
    {
        auto it = _systemMapByType.find(typeid(T));
        if (it != _systemMapByType.end()) {
            return static_cast<T *>(it->second);
        }

        return nullptr;
    }

    inline Span<System *> GetSystemList() {
        return { _systemList.begin(), _systemList.end() };
    }

private:

    List<System *> _systemList;

    Map<TypeIndex, System *> _systemMapByType;

}; // class Scene

RYME_API
void SetCurrentScene(Scene * scene);

RYME_API
Scene * GetCurrentScene();

} // namespace ryme

#endif // RYME_SCENE_HPP