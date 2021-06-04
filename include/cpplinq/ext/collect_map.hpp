#pragma once

#include "cpplinq/ext/container_detect.hpp"
#include <map>

namespace cpplinq {
namespace detail {

template<typename T, typename V, typename Compare, typename Alloc>
struct detect_container<std::map<T, V, Compare, Alloc>>
{
  using container_type = std::map<T, V, Compare, Alloc>;
  using value_type = typename container_type::value_type;

  static void insert(container_type& cont, value_type value) { cont.insert(std::move(value)); }
};

} // namespace detail
} // namespace cpplinq
