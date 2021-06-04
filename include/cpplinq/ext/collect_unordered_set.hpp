#pragma once

#include "cpplinq/ext/container_detect.hpp"
#include <unordered_set>

namespace cpplinq {
namespace detail {

template<typename T, typename Hash, typename KeyEq, typename Alloc>
struct detect_container<std::unordered_set<T, Hash, KeyEq, Alloc>>
{
  using container_type = std::unordered_set<T, Hash, KeyEq, Alloc>;
  using value_type = typename container_type::value_type;

  static void insert(container_type& cont, value_type value) { cont.insert(std::move(value)); }
};

} // namespace detail
} // namespace cpplinq
