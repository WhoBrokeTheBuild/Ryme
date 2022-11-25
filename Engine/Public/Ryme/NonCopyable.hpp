#ifndef RYME_NON_COPYABLE_HPP
#define RYME_NON_COPYABLE_HPP

#include <Ryme/Config.hpp>

namespace ryme {

class NonCopyable
{
protected:

    NonCopyable() = default;

    virtual ~NonCopyable() = default;

    NonCopyable(NonCopyable&) = delete;

    NonCopyable(const NonCopyable&) = delete;

    NonCopyable& operator=(NonCopyable&) = delete;

    NonCopyable& operator=(const NonCopyable&) = delete;

}; // class NonCopyable

} // namespace ryme

#endif // RYME_NON_COPYABLE_HPP