#ifndef RYME_UTF_HPP
#define RYME_UTF_HPP

#include <Ryme/Config.hpp>
#include <Ryme/String.hpp>

#include <optional>

namespace ryme {

namespace UTF {

RYME_API
bool IsValid(StringView str);

RYME_API
std::optional<size_t> GetLength(StringView str);

RYME_API
std::optional<U32String> ToUTF32(StringView str);

RYME_API
String ToUTF8(U32StringView u32str);

RYME_API
char32_t ToLower(char32_t codePoint);

RYME_API
char32_t ToUpper(char32_t codePoint);

#if defined(RYME_PLATFORM_WINDOWS)

    RYME_API
    WideString ToWideString(String str);

    RYME_API
    String ToUTF8(WideString str);

#endif

} // namespace UTF

} // namespace ryme

#endif // RYME_UTF_HPP