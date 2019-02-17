#ifndef CPPEG_META_HPP
#define CPPEG_META_HPP

#include "cppeg_common.hpp"
#include <optional>
#include <variant>
#include <type_traits>

CPPEG_NAMESPACE_OPEN

namespace meta {

template<typename T>
struct remove_optional {
    using type = T;
};

template<typename T>
struct remove_optional<std::optional<T>> {
    using type = T;
};

template<typename T>
using remove_optional_t = typename remove_optional<T>::type;

template<typename T, T I>
constexpr T integral_constant_value(std::integral_constant<T, I>) {
    return I;
}

template<typename T>
struct is_variant {
    constexpr static bool value = false;
};

template<typename ...T>
struct is_variant<std::variant<T...>> {
    constexpr static bool value = true;
};

template<typename T>
constexpr bool is_variant_v = is_variant<T>::value;


//-------------------------------------------------------------------
template<typename T>
struct is_optional {
    constexpr static bool value = false;
};

template<typename T>
struct is_optional<std::optional<T>> {
    constexpr static bool value = true;
};

template<typename T>
constexpr bool is_optional_v = is_optional<T>::value;


} // end namespace meta

CPPEG_NAMESPACE_CLOSE

#endif
