#include <Ryme/Log.hpp>

namespace ryme {

RYME_API
void LogMessage(StringView tag, StringView message)
{
    fmt::print("({}) {}\n", tag, message);
}

} // namespace ryme