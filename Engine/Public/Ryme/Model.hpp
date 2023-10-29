#ifndef RYME_MODEL_HPP
#define RYME_MODEL_HPP

#include <Ryme/Config.hpp>
#include <Ryme/Asset.hpp>
#include <Ryme/List.hpp>
#include <Ryme/Mesh.hpp>
#include <Ryme/Path.hpp>
#include <Ryme/Vertex.hpp>

#include <Ryme/JSON.hpp>

#include <Ryme/ThirdParty/vulkan.hpp>

namespace ryme {

class RYME_API Model : public Asset
{
public:

    Model(const Path& path, bool search = true);

    virtual ~Model();

    bool LoadFromFile(const Path& path, bool search = true);

    void Free() override;

    bool Reload() override;

    bool CanReload() const override {
        return true;
    }

    void Render(vk::CommandBuffer buffer);

private:

    bool LoadGLTF2(const Path& path, bool search);

    bool LoadOBJ(const Path& path, bool search);

    Path _path;

    List<Mesh> _meshList;

}; // class Model

} // namespace ryme

#endif // RYME_MODEL_HPP