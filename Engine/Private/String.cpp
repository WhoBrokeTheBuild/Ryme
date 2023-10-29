#include <Ryme/String.hpp>

namespace ryme {

RYME_API
List<String> Split(StringView str, String delim)
{
    List<String> strList;

    auto next = str.find(delim);
    while (next != String::npos) {
        strList.push_back(String(str.substr(0, next)));
        str = str.substr(next + 1);
        next = str.find(delim);
    }

    return strList;
}

RYME_API
String Join(List<String> strList, String delim)
{
    size_t totalSize = 0;
    for (const auto& str : strList) {
        totalSize += str.size() + delim.size();
    }

    String result;
    result.reserve(totalSize);

    for (const auto& str : strList) {
        result += str;
        result += delim;
    }

    return result;
}

RYME_API
String Join(List<StringView> strList, StringView delim)
{
    size_t totalSize = 0;
    for (const auto& str : strList) {
        totalSize += str.size() + delim.size();
    }

    String result;
    result.reserve(totalSize);

    for (const auto& str : strList) {
        result += str;
        result += delim;
    }

    return result;
}

} // namespace ryme