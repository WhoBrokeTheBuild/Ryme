#ifndef RYME_UTF_HPP
#define RYME_UTF_HPP

#include <Ryme/Config.hpp>
#include <Ryme/String.hpp>

#include <optional>

namespace ryme {

namespace UTF {

static const char32_t ReplacementCharacter = U'\xFFFD';

RYME_API
bool IsValid(StringView str);

RYME_API
size_t GetLength(StringView str);

RYME_API
U32String ToUTF32(StringView str, char32_t replace = ReplacementCharacter);

RYME_API
String ToUTF8(U32StringView u32str);

RYME_API
String CaseFold(StringView str);

RYME_API
U32String CaseFold(U32StringView u32str);

#if defined(RYME_PLATFORM_WINDOWS)

    RYME_API
    WideString ToWideString(String str);

    RYME_API
    String ToUTF8(WideString str);

#endif

} // namespace UTF

} // namespace ryme

#endif // RYME_UTF_HPP