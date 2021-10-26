#ifndef RYME_PLATFORM_HPP
#define RYME_PLATFORM_HPP

#if defined(_WIN32)

    // Windows
    #define RYME_PLATFORM_WINDOWS

#elif defined(__unix__)

    #if defined(__linux__)

        // Linux
        #define RYME_PLATFORM_LINUX

    #else

        #error Unsupported UNIX Operating System
        
    #endif

#else

    // Unknown
    #error Unsupported Operating System

#endif

#if defined(_MSC_VER)

    // Microsoft VisualStudio C/C++
    #define RYME_COMPILER_MSVC _MSC_VER

#elif defined(__clang__)

    // Apple Clang Compiler
    #define RYME_COMPILER_CLANG __clang__

#elif defined(__GNUC__) || defined(__GNUG__)
    
    // GNU Compiler Collection
    #define RYME_COMPILER_GCC __GNUC__

#elif defined(__ICC) || defined(__INTEL_COMPILER)

    // Intel Compiler
    #define RYME_COMPILER_ICC __INTEL_COMPILER

#else

    // Unknown
    #warning Unsupported Compiler

#endif

#if defined(RYME_COMPILER_MSVC)

    #define RYME_FUNCTION_NAME __FUNCSIG__

#elif defined(RYME_COMPILER_GCC) || defined(RYME_COMPILER_CLANG)

    #define RYME_FUNCTION_NAME __PRETTY_FUNCTION__

#else

    #define RYME_FUNCTION_NAME __func__

#endif

#if defined(NDEBUG)

    #define RYME_BUILD_RELEASE

#else

    #define RYME_BUILD_DEBUG

#endif

#if defined(RYME_PLATFORM_WINDOWS)

    #define RYME_PATH_SEPARATOR '\\'
    #define RYME_PATH_LIST_SEPARATOR ';'

#else

    #define RYME_PATH_SEPARATOR '/'
    #define RYME_PATH_LIST_SEPARATOR ':'

#endif

#endif // RYME_PLATFORM_HPP