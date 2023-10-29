#ifndef RYME_QUEUE_HPP
#define RYME_QUEUE_HPP

#include <Ryme/Config.hpp>

#include <deque>

namespace ryme {

template <class T>
using Queue = std::deque<T>;

} // namespace ryme

#endif // RYME_QUEUE_HPP