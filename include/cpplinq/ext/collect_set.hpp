#pragma once

#include "cpplinq/ext/container_detect.hpp"
#include <set>

namespace cpplinq {
namespace detail {

template<typename T, typename Compare, typename Allocator>
struct detect_container<std::set<T, Compare, Allocator>>
{
  using container_type = std::set<T, Compare, Allocator>;
  using value_type = typename container_type::value_type;

  static void insert(container_type& cont, value_type value) { cont.insert(std::move(value)); }
};

} // namespace detail
} // namespace cpplinq
