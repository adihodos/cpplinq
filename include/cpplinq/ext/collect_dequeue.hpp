#pragma once

#include "cpplinq/ext/container_detect.hpp"
#include <deque>

namespace cpplinq {
namespace detail {

template<typename T, typename Allocator>
struct detect_container<std::deque<T, Allocator>>
{
  using container_type = std::deque<T, Allocator>;
  using value_type = typename container_type::value_type;

  static void insert(container_type& cont, value_type value) { cont.push_back(std::move(value)); }
};

} // namespace detail
} // namespace cpplinq
