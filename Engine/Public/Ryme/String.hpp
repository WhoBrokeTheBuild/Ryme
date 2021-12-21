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

inline bool StartsWith(StringView str, StringView value)
{
    if (str.size() < value.size()) {
        return false;
    }

    return std::equal(
        value.begin(),
        value.end(),
        str.begin()
    );
}

inline bool EndsWith(StringView str, StringView value)
{
    if (str.size() < value.size()) {
        return false;
    }

    return std::equal(
        value.begin(),
        value.end(),
        str.end() - value.size()
    );
}

RYME_API
List<String> Split(StringView str, String delim);

RYME_API
String Join(List<String> strList, String delim = {});

} // namespace ryme

#endif // RYME_STRING_HPP