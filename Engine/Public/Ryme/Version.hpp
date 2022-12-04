#ifndef RYME_VERSION_HPP
#define RYME_VERSION_HPP

#include <Ryme/Config.hpp>
#include <Ryme/String.hpp>

#include <Ryme/ThirdParty/fmt.hpp>
#include <Ryme/ThirdParty/python.hpp>
#include <Ryme/ThirdParty/vulkan.hpp>

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
        FromString(str);
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
            Major, Minor, Patch
        );
    }

    inline uint32_t ToVkVersion() const
    {
        return VK_MAKE_VERSION(Major, Minor, Patch);
    }

    ///
    /// Parse a Version from a string in format major.minor.patch
    ///
    inline void FromString(const String& string);

    ///
    /// Compare two versions in order of Major, then Minor, then Patch
    ///
    /// @returns 0 if a == b, 1 if a > b, or -1 if a < b
    ///
    static int Compare(const Version& a, const Version& b);

public:

    static void ScriptInit(py::module);
    
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