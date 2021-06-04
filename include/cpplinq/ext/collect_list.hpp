#pragma once

#include "cpplinq/ext/container_detect.hpp"
#include <list>

namespace cpplinq {
namespace detail {

template<typename T, typename Allocator>
struct detect_container<std::list<T, Allocator>>
{
  using container_type = std::list<T, Allocator>;
  using value_type = typename container_type::value_type;

  static void insert(container_type& cont, value_type value) { cont.push_back(std::move(value)); }
};

} // namespace detail
} // namespace cpplinq
