#include <Ryme/String.hpp>
#include <Ryme/Containers.hpp>

#if defined(RYME_PLATFORM_WINDOWS)

    #include <Windows.h>

#endif // defined(RYME_PLATFORM_WINDOWS)

namespace ryme {

#if defined(RYME_PLATFORM_WINDOWS)

    RYME_API
    std::wstring ConvertUTF8ToWideString(String str)
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

    RYME_API
    String ConvertWideStringToUTF8(std::wstring wstr)
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

} // namespace ryme