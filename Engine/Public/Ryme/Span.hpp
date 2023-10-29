#ifndef RYME_SPAN_HPP
#define RYME_SPAN_HPP

#include <Ryme/Config.hpp>

#include <span>

namespace ryme {

template <class T, size_t N = std::dynamic_extent>
using Span = std::span<T, N>;

} // namespace ryme

#endif // RYME_SPAN_HPP