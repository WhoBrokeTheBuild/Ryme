#ifndef RYME_LOG_HPP
#define RYME_LOG_HPP

#include <Ryme/Config.hpp>
#include <Ryme/Path.hpp>
#include <Ryme/Version.hpp>

#include <fmt/core.h>
#include <fmt/ranges.h>
#include <fmt/chrono.h>
#include <fmt/os.h>
#include <fmt/color.h>

namespace ryme {

#define RYME_ANCHOR (fmt::format("{}:{}", ryme::Path(__FILE__).GetFilename().ToCString(), __LINE__))

RYME_API
void LogMessage(StringView tag, StringView message);

template <class... Args>
inline void Log(StringView tag, StringView format, const Args&... args) {
    LogMessage(tag, fmt::format(format, args...));
}

} // namespace ryme

#endif // RYME_LOG_HPP