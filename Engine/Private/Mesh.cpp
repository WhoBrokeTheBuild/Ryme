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

    if (ext == ".obj") {
        LoadOBJ(path, search);
    }
    else if (ext == ".gltf" || ext == ".glb") {
        LoadGLTF(path, search);
    }
    else {
        throw Exception("Unknown Mesh file format '{}'", ext);
    }

    _isLoaded = true;
    return true;
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

bool Mesh::LoadGLTF(const Path& path, bool search)
{
    return true;
}

bool Mesh::LoadOBJ(const Path& path, bool search)
{
    return true;
}

} // namespace ryme