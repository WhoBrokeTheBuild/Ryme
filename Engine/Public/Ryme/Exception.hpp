#ifndef RYME_EXCEPTION_HPP
#define RYME_EXCEPTION_HPP

#include <Ryme/Config.hpp>
#include <Ryme/String.hpp>

#include <fmt/core.h>

#include <stdexcept>

namespace ryme {

class RYME_API Exception : public std::exception
{
public:

    Exception(StringView message) noexcept
        : _message(message)
    { }

    template <class... Args>
    Exception(StringView format, const Args&... args) noexcept
        : _message(fmt::format(format, args...))
    { }

    const char * what() const noexcept override {
        return _message.c_str();
    }

private:

    String _message;

}; // class Exception

} // namespace ryme

#endif // RYME_EXCEPTION_HPP