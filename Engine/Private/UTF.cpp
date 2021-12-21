#include <Ryme/UTF.hpp>

#if defined(RYME_PLATFORM_WINDOWS)

    #include <Windows.h>

#endif // defined(RYME_PLATFORM_WINDOWS)

namespace ryme {

namespace UTF {

Tuple<size_t, char32_t> readCodePoint(StringView str, char32_t replace)
{
    char32_t codePoint = 0;

    int remaining = 0;
    for (size_t i = 0; i < str.size(); ++i) {
        const char& c = str[i];

        if (remaining == 0) {
            if ((c & 0b10000000) == 0) {
                codePoint = c;
            }
            else if ((c & 0b11100000) == 0b11000000) {
                codePoint = c & 0b00011111;
                remaining = 1;
            }
            else if ((c & 0b11110000) == 0b11100000) {
                codePoint = c & 0b00001111;
                remaining = 2;
            }
            else if ((c & 0b11111000) == 0b11110000) {
                codePoint = c & 0b00000111;
                remaining = 3;
            }
            else {
                return { i + 1, replace };
            }
        }
        else {
            if ((c & 0b11000000) != 0b10000000) {
                return { i + 1, replace };
            }

            codePoint = (codePoint << 6) | (c & 0b00111111);

            --remaining;
        }

        if (remaining == 0) {
            return { i + 1, codePoint };
        }
    }

    return { 0, 0 };
}

void writeCodePoint(String& str, char32_t codePoint)
{
    if (codePoint <= 0x7F) {
        str.push_back(codePoint);
    }
    else if (codePoint <= 0x7FF) {
        str.push_back(
            0b11000000 | ((codePoint >> 6) & 0b00011111)
        );
        str.push_back(
            0b10000000 | (codePoint & 0b00111111)
        );
    }
    else if (codePoint <= 0xFFFF) {
        str.push_back(
            0b11100000 | ((codePoint >> 12) & 0b00001111)
        );
        str.push_back(
            0b10000000 | ((codePoint >> 6) & 0b00111111)
        );
        str.push_back(
            0b10000000 | (codePoint & 0b00111111)
        );
    }
    else if (codePoint <= 0x10FFFF) {
        str.push_back(
            0b11110000 | ((codePoint >> 16) & 0b00000111)
        );
        str.push_back(
            0b10000000 | ((codePoint >> 12) & 0b00111111)
        );
        str.push_back(
            0b10000000 | ((codePoint >> 6) & 0b00111111)
        );
        str.push_back(
            0b10000000 | (codePoint & 0b00111111)
        );
    }
    else {
        writeCodePoint(str, ReplacementCharacter);
    }
}

bool IsValid(StringView str)
{
    char32_t codePoint = 0;

    size_t offset = 0;
    while (!str.empty()) {
        std::tie(offset, codePoint) = readCodePoint(str, 0);
        if (codePoint == 0) {
            return false;
        }

        str = str.substr(offset);
    }

    return true;
}

size_t GetLength(StringView str)
{
    size_t length = 0;
    char32_t codePoint = 0;

    size_t offset = 0;
    while (!str.empty()) {
        std::tie(offset, codePoint) = readCodePoint(str, 0);
        if (codePoint == 0) {
            return StringView::npos;
        }

        str = str.substr(offset);
        ++length;
    }

    return length;
}

U32String ToUTF32(StringView str, char32_t replace /*= ReplacementCharacter*/)
{
    U32String u32str;
    char32_t codePoint = 0;

    size_t offset = 0;
    while (offset < str.size()) {
        std::tie(offset, codePoint) = readCodePoint(str, replace);
        u32str.push_back(codePoint);
        str = str.substr(offset);
    }

    return u32str;
}

String ToUTF8(U32StringView u32str)
{
    String str;

    for (char32_t codePoint : u32str) {
        writeCodePoint(str, codePoint);
    }

    return str;
}

#include "UTF.CaseFolding.h"

RYME_API
String CaseFold(StringView str)
{
    return ToUTF8(CaseFold(ToUTF32(str)));
}

RYME_API
U32String CaseFold(U32StringView u32str)
{
    U32String folded;
    folded.reserve(u32str.size());

    bool found;
    for (auto& c : u32str) {
        found = false;

        for (const auto& mapping : _caseFoldMapping) {
            if (mapping.Uppercase == c) {
                found = true;

                folded.append(mapping.Lowercase);
            }
        }

        if (!found) {
            folded.push_back(c);
        }
    }

    return folded;
}

#if defined(RYME_PLATFORM_WINDOWS)

    WideString ConvertUTF8ToWideString(String str)
    {
        size_t maxSize = str.size() + 1;

        // Initialize to maximum potential size
        List<wchar_t> wide(maxSize);

        int result = MultiByteToWideChar(
            CP_UTF8, 0, 
            str.c_str(), -1, 
            wide.data(), wide.size());
        
        if (result <= 0) {
            return std::wstring();
        }

        return std::wstring(wide.data());
    }

    String ConvertWideStringToUTF8(WideString wstr)
    {
        // Each wide character can become between 1 and 4 bytes in UTF-8
        size_t maxSize = (wstr.size() * 4) + 1;

        // Initialize to maximum potential size
        List<char> utf8(maxSize);

        int result = WideCharToMultiByte(
            CP_UTF8, 0,
            wstr.c_str(), -1,
            utf8.data(), utf8.size(), 
            NULL, NULL);
            
        if (result <= 0) {
            return String();
        }

        return String(utf8.data());
    }

#endif // defined(RYME_PLATFORM_WINDOWS)

} // namespace UTF8

} // namespace ryme