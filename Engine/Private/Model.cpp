#include <Ryme/Model.hpp>
#include <Ryme/Exception.hpp>

namespace ryme {

RYME_API
Model::Model(const Path& path, bool search /*= true*/)
{
    LoadFromFile(path, search);
}

RYME_API
Model::~Model()
{
    Free();
}

RYME_API
bool Model::LoadFromFile(const Path& path, bool search /*= true*/)
{
    const Path& ext = path.GetExtension();

    if (ext == "obj") {
        _isLoaded = LoadOBJ(path, search);
    }
    else if (ext == "gltf" or ext == "glb") {
        _isLoaded = LoadGLTF2(path, search);
    }
    else {
        throw Exception("Unknown Model file format '{}'", ext);
    }

    return _isLoaded;
}

RYME_API
void Model::Free()
{
    _isLoaded = false;
}

RYME_API
bool Model::Reload()
{
    LoadFromFile(_path, false);
    return true;
}

RYME_API
void Model::Render(vk::CommandBuffer buffer)
{
    for (auto& mesh : _meshList) {
        mesh.GenerateCommands(buffer);
    }
}

} // namespace ryme