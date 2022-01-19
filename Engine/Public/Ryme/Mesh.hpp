#ifndef RYME_MESH_HPP
#define RYME_MESH_HPP

#include <Ryme/Config.hpp>
#include <Ryme/Asset.hpp>
#include <Ryme/Path.hpp>
#include <Ryme/Containers.hpp>

#include <Ryme/JSON.hpp>

#include <Ryme/ThirdParty/vulkan.hpp>

namespace ryme {

class Primitive;

class RYME_API Mesh : public Asset
{
public:

    Mesh(const Path& path, bool search = true);

    virtual ~Mesh();

    bool LoadFromFile(const Path& path, bool search = true);

    void Free() override;

    bool Reload() override;

    bool CanReload() const override {
        return true;
    }

private:

    bool LoadGLTF(const Path& path, bool search);

    bool LoadOBJ(const Path& path, bool search);

    Path _path;

    // List<Primitive> _primitiveList;

    vk::DescriptorSet _descriptorSet;

}; // class mesh

} // namespace ryme

#endif // RYME_MESH_HPP