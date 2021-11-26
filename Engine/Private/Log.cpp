#include <Ryme/Log.hpp>
#include <Ryme/Containers.hpp>

namespace ryme {

RYME_API
void LogMessage(StringView tag, StringView message)
{
    fmt::print("({}) {}\n", tag, message);
}

String FormatBytesHumanReadable(uint64_t bytes)
{
    static const Array<const char *, 7> suffixList = { "B", "KiB", "MiB", "GiB", "TiB", "PiB", "EiB" };

    size_t suffixIndex = 0;
    double temp = bytes;
    while (temp >= 1024.0) {
        ++suffixIndex;
        temp /= 1024;
    }

    if (suffixIndex >= suffixList.size()) {
        suffixIndex = 0;
        temp = bytes;
    }

    return fmt::format("{:.3} {}", temp, suffixList[suffixIndex]);
}

} // namespace ryme