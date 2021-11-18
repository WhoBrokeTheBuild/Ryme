#include <Ryme/Path.hpp>

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
}

List<Path> Path::ParsePathList(const String& str)
{
    List<Path> pathList;

    std::istringstream iss(str);
    String tmp;
    while (std::getline(iss, tmp, ListSeparator)) {
        pathList.push_back(Path(tmp));
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
    Normalize();
}

RYME_API
Path::Path(const StringView& str)
    : _path(str)
{
    Normalize();
}

RYME_API
Path::Path(const char * cstr)
    : _path(cstr)
{
    Normalize();
}

RYME_API
Path& Path::Append(const Path& rhs)
{
    // If path is empty, and doesn't have a trailing separator, append one
    if (rhs._path.empty()) {
        if (!_path.empty() && _path.back() != Separator) {
            _path += Separator;
        }
        return *this;
    }

    if (IsAbsolute() && rhs.IsAbsolute() && GetRootPath() != rhs.GetRootPath()) {
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

Path& Path::Concatenate(const Path& path)
{
    _path += path._path;
    return *this;
}

bool Path::Equals(const Path& rhs) const
{
    #if defined(RYME_PLATFORM_WINDOWS)

        return StringEqualCaseInsensitive(_path, rhs._path);

    #else

        return (_path == rhs._path);

    #endif
}

RYME_API
void Path::Normalize()
{
    if (_path.empty()) {
        return;
    }

    // TODO: Check valid UTF-8

    // TODO: Strip windows long filename marker "\\?\"

    #if defined(RYME_PLATFORM_WINDOWS)

        // Convert separators to native format
        for (auto& c : _path) {
            if (c == '/') {
                c = '\\';
            }
        }

    #endif

    auto begin = _path.begin();
    auto end = _path.end();

    // Skip double separators for paths starting like "\\server"
    if (_path.length() >= 2 && _path[0] == Separator && _path[1] == Separator) {
        begin += 2;
    }

    auto newEnd = std::unique(begin, end,
        [](char lhs, char rhs) {
            return (lhs == rhs && lhs == Separator);
        }
    );

    _path.erase(newEnd, end);
}

RYME_API
size_t Path::GetRootNameLength() const
{
    #if defined(RYME_PLATFORM_WINDOWS)

        // Check for windows drive letter, such as "C:"
        if (_path.length() >= 2 && _path[1] == ':') {
            char first = std::toupper(_path[0]);
            if (first >= 'A' && first <= 'Z') {
                return 2;
            }
        }

    #endif

    // Check for network path, such as "//server"
    if (_path.length() >= 3 && _path[0] == Separator && _path[1] == Separator) {
        if (_path[2] != Separator && std::isprint(_path[2])) {
            // Find first separator after server name
            size_t pos = _path.find_first_of(Separator, 3);
            if (pos == String::npos) {
                // The entire path is just a network share name
                return _path.length();
            }
            else {
                return pos;
            }
        }
    }

    // There is no root name
    return 0;
}

RYME_API
Path GetCurrentPath()
{
#if defined(RYME_PLATFORM_WINDOWS)

    std::unique_ptr<char> cwd(_getcwd(nullptr, 0));
    if (cwd) {
        return Path(String(cwd.get()));
    }

#else

    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd))) {
        return Path(cwd);
    }

#endif
    
    return Path();
}

} // namespace ryme