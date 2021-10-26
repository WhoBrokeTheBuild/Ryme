#ifndef RYME_LOG_HPP
#define RYME_LOG_HPP

#include <Ryme/Config.hpp>
#include <Ryme/Path.hpp>
#include <Ryme/Version.hpp>

#include <chrono>
#include <utility>

#include <fmt/core.h>
#include <fmt/ranges.h>
#include <fmt/chrono.h>
#include <fmt/os.h>
#include <fmt/color.h>

namespace ryme {

#define RYME_ANCHOR (fmt::format("{}:{}", ryme::Path(__FILE__).GetFilename().ToCString(), __LINE__))

#define RYME_BENCHMARK_START() \
    auto rymeBenchmarkStart = std::chrono::high_resolution_clock::now()

#define RYME_BENCHMARK_END()                                                    \
    ryme::Log(RYME_ANCHOR, "Function '{}' took {:.3} ms", RYME_FUNCTION_NAME,   \
        std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(  \
            std::chrono::high_resolution_clock::now() - rymeBenchmarkStart      \
        ).count())

RYME_API
void LogMessage(StringView tag, StringView message);

template <typename... Args>
inline void Log(StringView tag, StringView format, Args&&... args) {
    LogMessage(tag, fmt::vformat(format, fmt::make_format_args(std::forward<Args>(args)...)));
}

} // namespace ryme

#endif // RYME_LOG_HPP