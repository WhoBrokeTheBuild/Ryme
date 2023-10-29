#ifndef RYME_TUPLE_HPP
#define RYME_TUPLE_HPP

#include <Ryme/Config.hpp>

#include <tuple>

namespace ryme {

template <class... T>
using Tuple = std::tuple<T...>;

} // namespace ryme

#endif // RYME_TUPLE_HPP