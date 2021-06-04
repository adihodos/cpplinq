#pragma once

#include <deque>
#include <forward_list>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>
#include "cpplinq.hpp"

namespace cpplinq_ext {

namespace detail {

template <typename T> struct detect_container {};

template <typename T, typename Allocator>
struct detect_container<std::vector<T, Allocator>> {
  using container_type = std::vector<T, Allocator>;
  using value_type = typename container_type::value_type;

  static void insert(container_type &cont, value_type value) {
    cont.push_back(std::move(value));
  }
};

template <typename T, typename Allocator>
struct detect_container<std::deque<T, Allocator>> {
  using container_type = std::deque<T, Allocator>;
  using value_type = typename container_type::value_type;

  static void insert(container_type &cont, value_type value) {
    cont.push_back(std::move(value));
  }
};

template <typename T, typename Allocator>
struct detect_container<std::forward_list<T, Allocator>> {
  using container_type = std::forward_list<T, Allocator>;
  using value_type = typename container_type::value_type;

  static void insert(container_type &cont, value_type value) {
    cont.insert_after(std::end(cont), std::move(value));
  }
};

template <typename T, typename Allocator>
struct detect_container<std::list<T, Allocator>> {
  using container_type = std::list<T, Allocator>;
  using value_type = typename container_type::value_type;

  static void insert(container_type &cont, value_type value) {
    cont.push_back(std::move(value));
  }
};

template <typename T, typename Compare, typename Allocator>
struct detect_container<std::set<T, Compare, Allocator>> {
  using container_type = std::set<T, Compare, Allocator>;
  using value_type = typename container_type::value_type;

  static void insert(container_type &cont, value_type value) {
    cont.insert(std::move(value));
  }
};

template <typename T, typename Compare, typename Allocator>
struct detect_container<std::multiset<T, Compare, Allocator>> {
  using container_type = std::multiset<T, Compare, Allocator>;
  using value_type = typename container_type::value_type;

  static void insert(container_type &cont, value_type value) {
    cont.insert(std::move(value));
  }
};

template <typename T, typename V, typename Compare, typename Alloc>
struct detect_container<std::map<T, V, Compare, Alloc>> {
  using container_type = std::map<T, V, Compare, Alloc>;
  using value_type = typename container_type::value_type;

  static void insert(container_type &cont, value_type value) {
    cont.insert(std::move(value));
  }
};

template <typename T, typename V, typename Compare, typename Alloc>
struct detect_container<std::multimap<T, V, Compare, Alloc>> {
  using container_type = std::multimap<T, V, Compare, Alloc>;
  using value_type = typename container_type::value_type;

  static void insert(container_type &cont, value_type value) {
    cont.insert(std::move(value));
  }
};

template <typename T, typename V, typename Hash, typename KeyEq, typename Alloc>
struct detect_container<std::unordered_map<T, V, Hash, KeyEq, Alloc>> {
  using container_type = std::unordered_map<T, V, Hash, KeyEq, Alloc>;
  using value_type = typename container_type::value_type;

  static void insert(container_type &cont, value_type value) {
    cont.insert(std::move(value));
  }
};

template <typename T, typename V, typename Hash, typename KeyEq, typename Alloc>
struct detect_container<std::unordered_multimap<T, V, Hash, KeyEq, Alloc>> {
  using container_type = std::unordered_multimap<T, V, Hash, KeyEq, Alloc>;
  using value_type = typename container_type::value_type;

  static void insert(container_type &cont, value_type value) {
    cont.insert(std::move(value));
  }
};

template <typename T, typename Hash, typename KeyEq, typename Alloc>
struct detect_container<std::unordered_set<T, Hash, KeyEq, Alloc>> {
  using container_type = std::unordered_set<T, Hash, KeyEq, Alloc>;
  using value_type = typename container_type::value_type;

  static void insert(container_type &cont, value_type value) {
    cont.insert(std::move(value));
  }
};

template <typename T, typename Hash, typename KeyEq, typename Alloc>
struct detect_container<std::unordered_multiset<T, Hash, KeyEq, Alloc>> {
  using container_type = std::unordered_multiset<T, Hash, KeyEq, Alloc>;
  using value_type = typename container_type::value_type;

  static void insert(container_type &cont, value_type value) {
    cont.insert(std::move(value));
  }
};

template <typename TContainerType>
struct collect_builder : public cpplinq::detail::base_builder {
  template <typename TRange> TContainerType build(TRange range) {
    using container_insertion_helper = detect_container<TContainerType>;

    TContainerType container;

    while (range.next()) {
      container_insertion_helper::insert(container, range.front());
    }

    return container;
  }
};

} // namespace detail

template <typename TContainer>
CPPLINQ_INLINEMETHOD detail::collect_builder<TContainer>
collect() CPPLINQ_NOEXCEPT {
  return detail::collect_builder<TContainer>{};
}

} // namespace cpplinq_ext
