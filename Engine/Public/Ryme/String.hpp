#ifndef RYME_STRING_HPP
#define RYME_STRING_HPP

#include <Ryme/Config.hpp>
#include <Ryme/Containers.hpp>

#include <string>
#include <string_view>

namespace ryme {

using String = std::string;

using StringView = std::string_view;

using U32String = std::u32string;

using U32StringView = std::u32string_view;

using WideString = std::wstring;

using WideStringView = std::wstring_view;

inline bool StringContains(StringView haystack, StringView needle)
{
    return (haystack.find(needle) != String::npos);
}

inline constexpr size_t StringLength(const char * str)
{
    return std::char_traits<char>::length(str);
}

inline StringView StripLeft(StringView str)
{
    while (!str.empty() && std::isspace(str.front())) {
        str.remove_prefix(1);
    }
    return str;
}

inline StringView StripRight(StringView str)
{
    while (!str.empty() && std::isspace(str.back())) {
        str.remove_suffix(1);
    }
    return str;
}

inline StringView Strip(StringView str)
{
    str = StripLeft(str);
    str = StripRight(str);
    return str;
}

RYME_API
List<String> Split(StringView str, String delim);

RYME_API
String Join(List<String> strList, String delim);

RYME_API
String Join(List<StringView> strList, StringView delim);

} // namespace ryme

#endif // RYME_STRING_HPP