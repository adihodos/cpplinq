#pragma once

#include "cpplinq/ext/container_detect.hpp"
#include <forward_list>

namespace cpplinq {
namespace detail {

template<typename T, typename Allocator>
struct detect_container<std::forward_list<T, Allocator>>
{
  using container_type = std::forward_list<T, Allocator>;
  using value_type = typename container_type::value_type;

  static void insert(container_type& cont, value_type value) { cont.insert_after(std::end(cont), std::move(value)); }
};

} // namespace detail
} // namespace cpplinq
