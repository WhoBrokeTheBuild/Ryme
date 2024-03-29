#ifndef RYME_LOG_HPP
#define RYME_LOG_HPP

#include <Ryme/Config.hpp>
#include <Ryme/Path.hpp>
#include <Ryme/String.hpp>

#include <Ryme/ThirdParty/fmt.hpp>

#include <chrono>
#include <utility>

namespace ryme {

// Optimized version of Path::GetFilename to be used with __FILE__
inline StringView LogGetFilename(StringView filename)
{
    size_t pivot = filename.find_last_of(Path::Separator);
    return (
        pivot == StringView::npos
        ? filename
        : filename.substr(pivot + 1)
    );
}

#define RYME_ANCHOR (fmt::format("{}:{}", ryme::LogGetFilename(__FILE__), __LINE__))

#if defined(RYME_ENABLE_BENCHMARK)

    #define RYME_BENCHMARK_START() \
        auto rymeBenchmarkStart = std::chrono::high_resolution_clock::now()

    #define RYME_BENCHMARK_END()                                                    \
        ryme::Log(RYME_ANCHOR, "Function '{}' took {:.3} ms", RYME_FUNCTION_NAME,   \
            std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(  \
                std::chrono::high_resolution_clock::now() - rymeBenchmarkStart      \
            ).count())

#else

    #define RYME_BENCHMARK_START()

    #define RYME_BENCHMARK_END()

#endif

///
/// Log a message with a given tag
///
/// @param tag The prefix to the log message, wrapped in (), usually RYME_ANCHOR
/// @param message The message to log
///
RYME_API
void LogMessage(StringView tag, StringView message);

///
/// Format and Log a message with a given tag
///
/// @param tag The prefix to the log message, wrapped in (), usually RYME_ANCHOR
/// @param format The format string for fmt::format()
/// @param args... The remaining arguments to fmt::format()
///
template <typename... Args>
inline void Log(StringView tag, StringView format, Args&&... args) {
    LogMessage(tag, fmt::vformat(format, fmt::make_format_args(args...)));
}

///
/// Convert a byte count to a human readable number
///
/// 123456 would become 120.562 MiB
///
/// @param bytes The number of bytes
///
String FormatBytesHumanReadable(uint64_t bytes);

} // namespace ryme

#endif // RYME_LOG_HPP