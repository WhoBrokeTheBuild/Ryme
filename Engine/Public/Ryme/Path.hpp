#ifndef RYME_PATH_HPP
#define RYME_PATH_HPP

#include <Ryme/Config.hpp>
#include <Ryme/List.hpp>
#include <Ryme/String.hpp>
#include <Ryme/UTF.hpp>

#include <Ryme/ThirdParty/fmt.hpp>
#include <Ryme/ThirdParty/python.hpp>

namespace ryme {

class RYME_API Path
{
public:

    static const char Separator = RYME_PATH_SEPARATOR;

    static const char ListSeparator = RYME_PATH_LIST_SEPARATOR;

    static List<Path> ParsePathList(StringView str);

    Path() = default;

    Path(const Path& rhs);

    Path(const String& str);

    Path(const StringView& str);

    Path(const char * cstr);

    // Appends a new element to the path with a separator
    Path& Append(const Path& rhs);

    // Concatenates the paths without adding a separator
    Path& Concatenate(const Path& rhs);

    inline String ToString() const {
        return _path;
    }

    inline const char * ToCString() const {
        return _path.c_str();
    }

    inline bool IsEmpty() const {
        return _path.empty();
    }

    inline bool IsAbsolute() const {
        return HasRootDirectory();
    }

    inline bool IsRelative() const {
        return not HasRootDirectory();
    }

    inline bool HasRootName() const {
        return (getRootNameLength() > 0);
    }

    /// The root identifier on a filesystem with multiple roots, such as "C:" or "//server"
    inline Path GetRootName() const {
        return _path.substr(0, getRootNameLength());
    }

    inline bool HasRootDirectory() const {
        size_t rootNameLen = getRootNameLength();
        return (_path.length() > rootNameLen and _path[rootNameLen] == Separator);
    }

    inline Path GetRootDirectory() const {
        if (HasRootDirectory()) {
            return String(1, Separator);
        }
        return Path();
    }

    inline bool HasRootPath() const {
        return (HasRootName() or HasRootDirectory());
    }

    inline Path GetRootPath() const {
        return GetRootName() + GetRootDirectory();
    }

    inline Path GetParentPath() const {
        size_t pivot = _path.find_last_of(Separator);
        return (
            pivot <= getRootNameLength() or pivot == String::npos
            ? _path
            : _path.substr(0, pivot)
        );
    }

    inline Path GetFilename() const {
        size_t pivot = _path.find_last_of(Separator);
        return (
            pivot <= getRootNameLength() or pivot == String::npos
            ? _path
            : _path.substr(pivot + 1)
        );
    }

    inline Path GetStem() const {
        String filename = GetFilename().ToString();
        size_t pivot = filename.find_last_of('.');
        return (
            pivot == 0 or pivot == String::npos
            ? filename
            : filename.substr(0, pivot)
        );
    }

    inline Path GetExtension() const {
        String filename = GetFilename().ToString();
        size_t pivot = filename.find_last_of('.');
        return (
            pivot == 0 or pivot == String::npos
            ? String()
            : filename.substr(pivot + 1)
        );
    }

    inline Path& operator=(const Path& rhs) {
        _path = rhs._path;
        return *this;
    }

    inline Path& operator/=(const Path& rhs) {
        return Append(rhs);
    }

    inline Path& operator+=(const Path& rhs) {
        return Concatenate(rhs);
    }

    bool Equals(const Path& rhs) const;

    inline friend bool operator==(const Path& lhs, const Path& rhs) {
        return lhs.Equals(rhs);
    }

    inline friend bool operator!=(const Path& lhs, const Path& rhs) {
        return not lhs.Equals(rhs);
    }

    inline friend Path operator/(const Path& lhs, const Path& rhs)
    {
        Path tmp(lhs);
        tmp /= rhs;
        return tmp;
    }

    inline friend Path operator+(const Path& lhs, const Path& rhs)
    {
        Path tmp(lhs);
        tmp += rhs;
        return tmp;
    }

private:

    void normalize();

    size_t getRootNameLength() const;

    String _path;

public:

    static void ScriptInit(py::module);

}; // class Path

RYME_API
Path GetCurrentPath();

RYME_API
List<Path> GetAssetPathList();

} // namespace ryme

template <>
struct fmt::formatter<ryme::Path> : public fmt::formatter<std::string_view>
{
    template <typename FormatContext>
    auto format(const ryme::Path& path, FormatContext& ctx) {
        return formatter<std::string_view>::format(std::string_view(path.ToString()), ctx);
    }
};

#endif // RYME_PATH_HPP