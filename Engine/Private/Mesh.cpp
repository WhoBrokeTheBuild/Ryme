#include <Ryme/Mesh.hpp>
#include <Ryme/Exception.hpp>

namespace ryme {

Mesh::Mesh(const Path& path, bool search /*= true*/)
{
    LoadFromFile(path, search);
}

Mesh::~Mesh()
{
    Free();
}

bool Mesh::LoadFromFile(const Path& path, bool search /*= true*/)
{
    const Path& ext = path.GetExtension();

    if (ext == "obj") {
        _isLoaded = LoadOBJ(path, search);
    }
    else if (ext == "gltf" || ext == "glb") {
        _isLoaded = LoadGLTF2(path, search);
    }
    else {
        throw Exception("Unknown Mesh file format '{}'", ext);
    }

    return _isLoaded;
}

void Mesh::Free()
{
    _isLoaded = false;
}

bool Mesh::Reload()
{
    LoadFromFile(_path, false);
    return true;
}

} // namespace ryme