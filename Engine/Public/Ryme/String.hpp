#ifndef RYME_STRING_HPP
#define RYME_STRING_HPP

#include <Ryme/Config.hpp>

#include <string>
#include <string_view>

namespace ryme {

using String = std::string;

using StringView = std::string_view;

inline bool StringEqualCaseInsensitive(StringView a, StringView b)
{
    // TODO: UTF-8
    return std::equal(
        a.begin(), a.end(),
        b.begin(), b.end(),
        [](char a, char b) {
            return std::tolower(a) == std::tolower(b);
        }
    );
}

#if defined(RYME_PLATFORM_WINDOWS)

    RYME_API
    std::wstring ConvertUTF8ToWideString(String str);

    RYME_API
    String ConvertWideStringToUTF8(std::wstring str);

#endif

} // namespace ryme

#endif // RYME_STRING_HPP