#include <Ryme/Path.hpp>
#include <Ryme/Exception.hpp>

#include <algorithm>
#include <cstdio>
#include <climits>
#include <sstream>

#include <pybind11/stl.h>
#include <pybind11/operators.h>

#if defined(RYME_PLATFORM_WINDOWS)

    #include <direct.h>

#else

    #include <unistd.h>

#endif

namespace ryme {

RYME_API
List<Path> Path::ParsePathList(StringView str)
{
    List<Path> pathList;

    auto next = str.find(ListSeparator);
    while (next != String::npos) {
        pathList.push_back(Path(str.substr(0, next)));
        str = str.substr(next + 1);
        next = str.find(ListSeparator);
    }

    return pathList;
}

RYME_API
Path::Path(const Path& rhs)
    : _path(rhs._path)
{ }

RYME_API
Path::Path(const String& str)
    : _path(str)
{
    normalize();
}

RYME_API
Path::Path(const StringView& str)
    : _path(str)
{
    normalize();
}

RYME_API
Path::Path(const char * cstr)
    : _path(cstr)
{
    normalize();
}

RYME_API
Path& Path::Append(const Path& rhs)
{
    if (IsAbsolute() and rhs.IsAbsolute()) {
        // Unable to append absolute paths
        _path = rhs._path;
        return *this;
    }

    if (_path.back() != Separator) {
        _path += Separator;
    }

    _path += rhs._path;
    return *this;
}

RYME_API
Path& Path::Concatenate(const Path& rhs)
{
    _path += rhs._path;
    return *this;
}

RYME_API
bool Path::Equals(const Path& rhs) const
{
    #if defined(RYME_PLATFORM_WINDOWS)

        auto pathA = UTF::CaseFold(_path);
        auto pathB = UTF::CaseFold(rhs._path);
        
        return (pathA == pathB);

    #else

        return (_path == rhs._path);

    #endif
}

RYME_API
void Path::normalize()
{
    if (_path.empty()) {
        return;
    }

    if (not UTF::IsValid(_path)) {
        _path.clear();
        throw Exception("Path is not a valid Unicode string");
    }

    // TODO: Strip windows long filename marker "\\?\"

    #if defined(RYME_PLATFORM_WINDOWS)

        // Convert separators to native format
        for (auto& c : _path) {
            if (c == '/') {
                c = '\\';
            }
        }

    #endif
    
    // TODO: Handle network paths

    // Strip duplicate slashes, e.g. /path//to/file
    auto newEnd = std::unique(
        _path.begin(), 
        _path.end(),
        [](char lhs, char rhs) {
            return (lhs == rhs and lhs == Separator);
        }
    );

    _path.erase(newEnd, _path.end());
}

RYME_API
size_t Path::getRootNameLength() const
{
    #if defined(RYME_PLATFORM_WINDOWS)

        // Check for windows drive letter, such as "C:"
        if (_path.length() >= 2 and _path[1] == ':') {
            char first = std::toupper(_path[0]);
            if (first >= 'A' and first <= 'Z') {
                return 2;
            }
        }

    #endif

    // TODO: Handle network paths

    // There is no root name
    return 0;
}

RYME_API
Path GetCurrentPath()
{
#if defined(RYME_PLATFORM_WINDOWS)

    std::unique_ptr<char> cwd(_getcwd(nullptr, 0));
    if (cwd) {
        return Path(cwd.get());
    }

#else

    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd))) {
        return Path(cwd);
    }

#endif
    
    return Path();
}

RYME_API
void Path::ScriptInit(py::module m)
{
    py::class_<Path>(m, "Path")

        .def(py::init())

        .def(py::init<const String&>(),
            py::arg("str"))

        .def_static("ParsePathList", &Path::ParsePathList)

        .def("IsEmpty", &Path::IsEmpty)
        .def("IsAbsolute", &Path::IsAbsolute)
        .def("IsRelative", &Path::IsRelative)
        .def("HasRootName", &Path::HasRootName)
        .def("GetRootName", &Path::GetRootName)
        .def("HasRootDirectory", &Path::HasRootDirectory)
        .def("GetRootDirectory", &Path::GetRootDirectory)
        .def("HasRootPath", &Path::HasRootPath)
        .def("GetRootPath", &Path::GetRootPath)
        .def("GetParentPath", &Path::GetParentPath)
        .def("GetFilename", &Path::GetFilename)
        .def("GetStem", &Path::GetStem)
        .def("GetExtension", &Path::GetExtension)

        .def(py::self == py::self)
        .def(py::self != py::self)
        .def(py::self /= py::self)
        .def("__itruediv__", 
            [](Path& lhs, py::str rhs) {
                lhs /= Path(rhs.cast<StringView>());
            })
        .def(py::self += py::self)
        .def("__iadd__", 
            [](Path& lhs, py::str rhs) {
                lhs += Path(rhs.cast<StringView>());
            })
        .def(py::self / py::self)
        .def("__truediv__", 
            [](Path& lhs, py::str rhs) {
                return lhs / Path(rhs.cast<StringView>());
            })
        .def(py::self + py::self)
        .def("__add__", 
            [](Path& lhs, py::str rhs) {
                return lhs + Path(rhs.cast<StringView>());
            })

        .def("__str__", &Path::ToString)

        .def("__repr__", 
            [](const Path& p) {
                return fmt::format(
                    "ryme.Path('{}')",
                    p.ToString()
                );
            });

    m.def("GetCurrentPath", GetCurrentPath);
    m.def("GetAssetPathList", GetAssetPathList);
}

RYME_API
List<Path> GetAssetPathList()
{
    static List<Path> assetPathList;
    
    if (assetPathList.empty()) {
        char * env = getenv("RYME_ASSET_PATH");
        if (env) {
            assetPathList = Path::ParsePathList(env);
        }
    }

    return assetPathList;
}

} // namespace ryme