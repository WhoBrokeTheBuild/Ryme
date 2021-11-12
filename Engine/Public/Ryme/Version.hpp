#ifndef RYME_VERSION_HPP
#define RYME_VERSION_HPP

#include <Ryme/Config.hpp>
#include <Ryme/String.hpp>
#include <Ryme/Script.hpp>

#include <fmt/format.h>

#include <regex>
#include <tuple>


namespace ryme {

///
/// Implementation of a Semantic Version, with only Major, Minor, and Patch
///
struct RYME_API Version
{
    ///
    /// The major component of the version major.x.x
    ///
    int Major;

    ///
    /// The minor component of the version x.minor.x
    ///
    int Minor;

    ///
    /// The patch component of the version x.x.patch
    ///
    int Patch;

    static void ScriptInit(py::module m);

    ///
    /// Construct a Version from three components
    ///
    /// @param major The major component of the version major.x.x
    /// @param minor The minor component of the version x.minor.x
    /// @param patch The patch component of the version x.x.patch
    ///
    Version(int major = 0, int minor = 0, int patch = 0)
        : Major(major)
        , Minor(minor)
        , Patch(patch)
    { }

    ///
    /// Parse a Version from a string in format major.minor.patch
    ///
    Version(const String& str)
    {
        ParseString(str);
    }

    Version(const Version& rhs)
        : Major(rhs.Major)
        , Minor(rhs.Minor)
        , Patch(rhs.Patch)
    { }

    inline Version& operator=(const Version& rhs)
    {
        Major = rhs.Major;
        Minor = rhs.Minor;
        Patch = rhs.Patch;
        return *this;
    }

    inline friend bool operator==(const Version& lhs, const Version& rhs)
    {
        return (Version::Compare(lhs, rhs) == 0);
    }

    inline friend bool operator!=(const Version& lhs, const Version& rhs)
    {
        return (Version::Compare(lhs, rhs) != 0);
    }

    inline friend bool operator>(const Version& lhs, const Version& rhs)
    {
        return (Version::Compare(lhs, rhs) == 1);
    }

    inline friend bool operator>=(const Version& lhs, const Version& rhs)
    {
        int res = Version::Compare(lhs, rhs);
        return (res == 0 || res == 1);
    }

    inline friend bool operator<(const Version& lhs, const Version& rhs)
    {
        return (Version::Compare(lhs, rhs) == -1);
    }

    inline friend bool operator<=(const Version& lhs, const Version& rhs)
    {
        int res = Version::Compare(lhs, rhs);
        return (res == 0 || res == -1);
    }

    ///
    /// @return A version string formatted as major.minor.patch
    ///
    inline String ToString() const
    {
        return fmt::format("{}.{}.{}", 
            Major, Minor, Patch);
    }

    inline operator String() const
    {
        return ToString();
    }

    ///
    /// Parse a Version from a string in format major.minor.patch
    ///
    inline void ParseString(const String& string)
    {
        std::smatch match;
        std::regex regex("([0-9])+\\.([0-9]+)\\.([0-9]+)");
        std::regex_search(string, match, regex);

        if (match.size() == 4) {
            Patch = std::strtol(match[3].str().c_str(), nullptr, 10);
            Minor = std::strtol(match[2].str().c_str(), nullptr, 10);
            Major = std::strtol(match[1].str().c_str(), nullptr, 10);
        }
    }

    ///
    /// Compare two versions in order of Major, then Minor, then Patch
    ///
    /// @returns 0 if a == b, 1 if a > b, or -1 if a < b
    ///
    inline static int Compare(const Version& a, const Version& b)
    {
        auto cmp = [](int a, int b)
        {
            if (a == b) {
                return 0;
            }
            if (a > b) {
                return 1;
            }
            return -1;
        };

        int result = cmp(a.Major, b.Major);
        if (result == 0) {
            result = cmp(a.Minor, b.Minor);
            if (result == 0) {
                result = cmp(a.Patch, b.Patch);
            }
        }
        return result;
    }

}; // struct Version

} // namespace ryme

template <>
struct fmt::formatter<ryme::Version> : public fmt::formatter<std::string_view>
{
    template <typename FormatContext>
    auto format(const ryme::Version& version, FormatContext& ctx) {
        return formatter<std::string_view>::format(std::string_view(version.ToString()), ctx);
    }
};

#endif // RYME_VERSION_HPP