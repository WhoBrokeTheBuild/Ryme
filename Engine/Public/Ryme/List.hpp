#ifndef RYME_LIST_HPP
#define RYME_LIST_HPP

#include <Ryme/Config.hpp>

#include <algorithm>
#include <vector>

namespace ryme {

template <class T>
using List = std::vector<T>;

template <class T>
inline bool ListContains(const List<T>& haystack, const T& needle)
{
    auto it = std::find(std::begin(haystack), std::end(haystack), needle);
    return (it != haystack.end());
}

template <class T>
inline bool ListRemove(List<T>& haystack, const T& needle)
{
    auto it = std::find(std::begin(haystack), std::end(haystack), needle);
    if (it == haystack.end()) {
        return false;
    }

    haystack.erase(it);
    return true;
}

} // namespace ryme

#endif // RYME_LIST_HPP