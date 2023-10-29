#ifndef RYME_ARRAY_HPP
#define RYME_ARRAY_HPP

#include <Ryme/Config.hpp>

#include <array>

namespace ryme {

template <class T, std::size_t N>
using Array = std::array<T, N>;

} // namespace ryme

#endif // RYME_ARRAY_HPP