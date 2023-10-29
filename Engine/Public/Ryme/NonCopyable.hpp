#ifndef RYME_UTIL_NON_COPYABLE_HPP
#define RYME_UTIL_NON_COPYABLE_HPP

#include <Ryme/Config.hpp>

namespace ryme {

class NonCopyable
{
protected:

    NonCopyable() = default;

    NonCopyable(NonCopyable&&) = default;

    virtual ~NonCopyable() = default;

    NonCopyable(NonCopyable&) = delete;

    NonCopyable(const NonCopyable&) = delete;

    NonCopyable& operator=(NonCopyable&) = delete;

    NonCopyable& operator=(const NonCopyable&) = delete;

}; // class NonCopyable

} // namespace ryme

#endif // RYME_UTIL_NON_COPYABLE_HPP