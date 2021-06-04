#pragma once

#include "cpplinq/cpplinq.hpp"
#include "cpplinq/ext/container_detect.hpp"
#include <type_traits>

namespace cpplinq {
namespace detail {

template<typename TContainerType>
struct collect_builder : public base_builder
{
  TContainerType _container;

  collect_builder() = default;

  explicit collect_builder(TContainerType&& cont)
    : _container{ std::forward<TContainerType>(cont) }
  {}

  template<typename TRange>
  TContainerType build(TRange range)
  {
    using container_insertion_helper = detect_container<TContainerType>;

    while (range.next()) {
      container_insertion_helper::insert(_container, range.front());
    }

    return std::move(_container);
  }
};

} // namespace detail

template<typename F, typename... FArgs>
CPPLINQ_INLINEMETHOD detail::collect_builder<typename std::result_of<F && (FArgs && ...)>::type>
collect(F&& f, FArgs&&... args)
{
  using container_type = typename std::result_of<F && (FArgs && ...)>::type;
  return detail::collect_builder<container_type>{ f(std::forward<FArgs>(args)...) };
}

template<typename TContainer>
CPPLINQ_INLINEMETHOD detail::collect_builder<TContainer>
collect() CPPLINQ_NOEXCEPT
{
  return detail::collect_builder<TContainer>{};
}

} // namespace cpplinq
