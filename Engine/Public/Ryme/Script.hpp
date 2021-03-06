#ifndef RYME_SCRIPT_HPP
#define RYME_SCRIPT_HPP

#include <Ryme/Config.hpp>
#include <Ryme/String.hpp>

#include <Ryme/ThirdParty/fmt.hpp>
#include <Ryme/ThirdParty/python.hpp>

namespace ryme {

namespace Script {

RYME_API
void Init();

RYME_API
void Term();

} // namespace Script

} // namespace ryme

template<>
struct fmt::formatter<py::str> : public fmt::formatter<std::string_view>
{
    template <typename FormatContext>
    auto format(const py::str& str, FormatContext& ctx) {
        return formatter<std::string_view>::format(str.cast<std::string_view>(), ctx);
    }
};

#endif // RYME_SCRIPT_HPP