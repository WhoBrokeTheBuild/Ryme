#ifndef RYME_CONTAINERS_MAP_HPP
#define RYME_CONTAINERS_MAP_HPP

#include <unordered_map>

namespace ryme {

template <class K, class V>
using Map = std::unordered_map<K, V>;

} // namespace ryme

#endif // RYME_CONTAINERS_MAP_HPP