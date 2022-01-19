#ifndef RYME_CONTAINERS_HPP
#define RYME_CONTAINERS_HPP

#include <Ryme/Config.hpp>

#include <algorithm>
#include <array>
#include <deque>
#include <set>
#include <span>
#include <tuple>
#include <unordered_map>
#include <vector>

namespace ryme {

// TODO: Expand to include all template arguments

template <class T, size_t N>
using Array = std::array<T, N>;

template <class T>
using Queue = std::deque<T>;

template <class T>
using Set = std::set<T>;

template <class T, size_t N = std::dynamic_extent>
using Span = std::span<T, N>;

template <class... T>
using Tuple = std::tuple<T...>;

template <class K, class V>
using Map = std::unordered_map<K, V>;

template <class T>
using List = std::vector<T>;

template <class T>
inline bool ListContains(const List<T>& list, const T& value)
{
    auto it = std::find(std::begin(list), std::end(list), value);
    return (it != list.end());
}

} // namespace ryme

#endif // RYME_CONTAINERS_HPP