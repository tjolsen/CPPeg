#ifndef CPPEG_HELPERS_HPP
#define CPPEG_HELPERS_HPP

#include "cppeg_common.hpp"
#include "rule.hpp"
#include "tmpl.hpp"
#include <string_view>

CPPEG_NAMESPACE_OPEN

namespace helpers {

template<template<typename...> typename CTOR, typename... L, typename R,
         int... I>
auto ctor_splat(std::tuple<L...> const &lhs, Rule<R> const &rhs,
                tmpl::value_list<I...>) {
    return CTOR<L..., R>(std::get<I>(lhs)..., rhs);
}

template<template<typename...> typename CTOR, typename L, typename... R,
         int... I>
auto ctor_splat(Rule<L> const &lhs, std::tuple<R...> const &rhs,
                tmpl::value_list<I...>) {
    return CTOR<L, R...>(lhs, std::get<I>(rhs)...);
}


//--------------------------------------------------------------------------

template <class T>
constexpr
std::string_view
type_name()
{
    using namespace std;
#ifdef __clang__
    string_view p = __PRETTY_FUNCTION__;
    string_view key = "with T = ";
    auto start = p.find(key);
    return string_view(p.data() + start + key.size(), p.find(';', start) - start - key.size());
    //return string_view(p.data() + 34, p.size() - 34 - 1);
#elif defined(__GNUC__)
    string_view p = __PRETTY_FUNCTION__;
#  if __cplusplus < 201402
    return string_view(p.data() + 36, p.size() - 36 - 1);
#  else
    string_view key = "with T = ";
    auto start = p.find(key);
    return string_view(p.data() + start + key.size(), p.find(';', start) - start - key.size());
#  endif
#elif defined(_MSC_VER)
    string_view p = __FUNCSIG__;
    return string_view(p.data() + 84, p.size() - 84 - 7);
#endif

    
}












} // end namespace helpers

CPPEG_NAMESPACE_CLOSE

#endif
