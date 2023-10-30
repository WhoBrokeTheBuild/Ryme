#ifndef RYME_EXCEPTION_HPP
#define RYME_EXCEPTION_HPP

#include <Ryme/Config.hpp>
#include <Ryme/String.hpp>

#include <Ryme/ThirdParty/fmt.hpp>

#include <stdexcept>
#include <utility>

namespace ryme {

class RYME_API Exception : public std::exception
{
public:

    Exception(StringView message) noexcept
        : _message(message)
    { }

    template <class... Args>
    Exception(StringView format, Args&&... args) noexcept
        : _message(fmt::vformat(format, fmt::make_format_args(args...)))
    { }

    const char * what() const noexcept override {
        return _message.c_str();
    }

private:

    String _message;

}; // class Exception

} // namespace ryme

#endif // RYME_EXCEPTION_HPP