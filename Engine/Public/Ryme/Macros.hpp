#ifndef RYME_MACROS_HPP
#define RYME_MACROS_HPP

#include <Ryme/Platform.hpp>

#define RYME_DISALLOW_COPY_AND_ASSIGN(TypeName) \
    TypeName(const TypeName&) = delete; \
    TypeName& operator=(const TypeName&) = delete;

#define _RYME_STRINGIFY(x) #x
#define RYME_STRINGIFY(x) _RYME_STRINGIFY(x)

#if defined(RYME_COMPILER_MSVC)

    #define RYME_DISABLE_WARNINGS() \
        __pragma(warning(push, 0))
    
    #define RYME_ENABLE_WARNINGS() \
        __pragma(warning(pop))

#elif defined(RYME_COMPILER_CLANG)

    #define RYME_DISABLE_WARNINGS() \
        _Pragma("clang diagnostic push") \
        _Pragma("clang diagnostic ignored \"-Wall\"")
    
    #define RYME_ENABLE_WARNINGS() \
        _Pragma("clang diagnostic pop")

#elif defined(RYME_COMPILER_GCC)

    #define RYME_DISABLE_WARNINGS() \
        _Pragma("GCC diagnostic push") \
        _Pragma("GCC diagnostic ignored \"-Wall\"")
    
    #define RYME_ENABLE_WARNINGS() \
        _Pragma("GCC diagnostic pop")

#else

    #define RYME_DISABLE_WARNINGS()

    #define RYME_ENABLE_WARNINGS()

#endif

#endif // RYME_MACROS_HPP