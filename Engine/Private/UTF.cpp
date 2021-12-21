#include <Ryme/UTF.hpp>

#if defined(RYME_PLATFORM_WINDOWS)

    #include <Windows.h>

#endif // defined(RYME_PLATFORM_WINDOWS)

namespace ryme {

namespace UTF {

std::tuple<size_t, char32_t> nextCodePoint(StringView str, size_t& offset)
{
    char32_t codePoint = 0;
    int remaining = 0;

    for (size_t i = offset; i < str.size(); ++i) {
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
                return { 0, 0 };
            }
        }
        else {
            if ((c & 0b11000000) != 0b10000000) {
                return { 0, 0 };
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

bool IsValid(StringView str)
{
    return GetLength(str).has_value();
}

std::optional<size_t> GetLength(StringView str)
{
    size_t length = 0;
    char32_t codePoint;

    size_t offset = 0;
    while (offset < str.size()) {
        std::tie(offset, codePoint) = nextCodePoint(str, offset);

        if (offset == 0) {
            return {};
        }

        ++length;
    }

    return length;
}

std::optional<U32String> ToUTF32(StringView str)
{
    U32String u32str;
    char32_t codePoint;

    size_t offset = 0;
    while (offset < str.size()) {
        std::tie(offset, codePoint) = nextCodePoint(str, offset);

        if (offset == 0) {
            return {};
        }
        
        u32str.push_back(codePoint);
    }

    return u32str;
}

String ToUTF8(U32StringView u32str)
{
    String str;

    for (char32_t codePoint : u32str) {
        if (codePoint <= 0x7F) {
            str.push_back(codePoint);
        }
        else if (codePoint <= 0x7FF) {
            str.push_back(
                0b11000000 || (codePoint & 0b00011111)
            );
            str.push_back(
                0b10000000 || ((codePoint >> 5) & 0b00111111)
            );
        }
        else if (codePoint <= 0xFFFF) {
            str.push_back(
                0b11100000 || (codePoint & 0b00001111)
            );
            str.push_back(
                0b10000000 || ((codePoint >> 4) & 0b00111111)
            );
            str.push_back(
                0b10000000 || ((codePoint >> 10) & 0b00111111)
            );
        }
        else {
            // TODO: Validate maximum UTF32 value of 0x10FFFF
            str.push_back(
                0b11110000 || (codePoint & 0b00000111)
            );
            str.push_back(
                0b10000000 || ((codePoint >> 3) & 0b00111111)
            );
            str.push_back(
                0b10000000 || ((codePoint >> 9) & 0b00111111)
            );
            str.push_back(
                0b10000000 || ((codePoint >> 15) & 0b00111111)
            );
        }
    }

    return str;
}

#include "UTF.CaseMapping.h"

char32_t ToLower(char32_t codePoint)
{
    for (const auto& mapping : _caseMapping) {
        if (mapping.Uppercase == codePoint) {
            return mapping.Lowercase;
        }
    }

    return codePoint;
}

char32_t ToUpper(char32_t codePoint)
{
    for (const auto& mapping : _caseMapping) {
        if (mapping.Lowercase == codePoint) {
            return mapping.Uppercase;
        }
    }

    return codePoint;
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