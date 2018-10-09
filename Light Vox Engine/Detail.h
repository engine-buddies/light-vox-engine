#pragma once
#include "stdafx.h"
#include <type_traits>

/* Detector for beautiful SFINAE */
namespace detail {

    template <template <typename> typename Op, typename T, typename = void>
    struct is_detected : std::false_type {};
    template <template <typename> typename Op, typename T>
    struct is_detected<Op, T, std::void_t<Op<T>>> : std::true_type {};

} // namespace detail

template <template <typename> typename Op, typename T>
static constexpr bool is_detected_v = detail::is_detected<Op, T>::value;

/* Engine provided member function "look ups". */
namespace detail {

    template <class U>
    using has_init = decltype(std::declval<U>().init());
    template <class U>
    using has_update = decltype(std::declval<U>().update(std::declval<float>()));

} // namespace detail


