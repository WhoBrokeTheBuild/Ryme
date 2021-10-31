#ifndef RYME_CONTAINERS_HPP
#define RYME_CONTAINERS_HPP

#include <Ryme/Config.hpp>

#include <array>
#include <deque>
#include <set>
#include <span>
#include <unordered_map>
#include <vector>

namespace ryme {

template <class T, size_t N>
using Array = std::array<T, N>;

template <class T, size_t N = std::dynamic_extent>
using Span = std::span<T, N>;

template <class T>
using Queue = std::deque<T>;

template <class T>
using Set = std::set<T>;

template <class K, class V>
using Map = std::unordered_map<K, V>;

template <class T>
using List = std::vector<T>;

} // namespace ryme

#endif // RYME_CONTAINERS_HPP