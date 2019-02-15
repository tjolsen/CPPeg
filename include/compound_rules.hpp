#ifndef CPPEG_COMPOUND_RULES
#define CPPEG_COMPOUND_RULES

#include "cppeg_common.hpp"
#include "meta.hpp"
#include "rule.hpp"

#include "tmpl.hpp"

#include <tuple>
#include <type_traits>

CPPEG_NAMESPACE_OPEN

/**
 * parse succeeds only if each subrule parse succeeds
 */
template<typename... Subrules>
struct AndRule : public Rule<AndRule<Subrules...>> {
    AndRule(Rule<Subrules> const &... subrules)
        : subrules(std::forward_as_tuple(subrules.self()...)) {}

    template<typename T>
    auto parse_impl(InputStream<T> &in) {

        constexpr auto types   = tmpl::type_list<Subrules...>{};
        constexpr auto indices = tmpl::arithmetic_sequence<types.size()>();

        using optional_parse_result_types = tmpl::type_list<
            std::decay_t<decltype(std::declval<Subrules>().parse(in))>...>;

        constexpr auto parse_result_type_list =
            tmpl::transform(optional_parse_result_types{}, [](auto &&x) {
                using xT = tmpl::head_type_t<decltype(x)>;
                return tmpl::type_list<meta::remove_optional_t<xT>>{};
            });

	//compute the final return type
        using return_tuple_type =
            std::decay_t<decltype(tmpl::as_tuple(parse_result_type_list))>;
        std::optional<return_tuple_type> ret;


	
        // make a tuple of optionals to hold intermediate parses
        auto tmp_ret = tmpl::as_tuple(optional_parse_result_types{});

        tmpl::for_each(tmpl::zip(types, indices), [&](auto &&tv_list) {
            auto tv    = tmpl::unbox(tv_list);
            using type = typename std::decay_t<decltype(tv)>::type_t;
            constexpr std::size_t I = tv.value();

            std::get<I>(tmp_ret) = std::get<I>(subrules).parse(in);
        });

	//Test if all parses succeeded.
        bool all_has_value = true;
        tmpl::for_each(indices, [&](auto x) {
            constexpr auto I = static_cast<std::size_t>(tmpl::unbox(x));
            all_has_value = all_has_value && std::get<I>(tmp_ret).has_value();
        });

        if (all_has_value) {
            return_tuple_type parse_results;
            tmpl::for_each(indices, [&](auto x) {
                constexpr auto I = static_cast<std::size_t>(tmpl::unbox(x));
                std::get<I>(parse_results) =
                    std::move(std::get<I>(tmp_ret).value());
            });
            ret = parse_results;
        }

        return ret;
    }


    // Rules are very lightweight by design. Store them by value
    // to avoid reference lifetime issues.
    using storage_type = std::tuple<Subrules...>;
    storage_type subrules;
};

template<typename... Subrules>
auto And(Rule<Subrules> const &... subrules) {
    return AndRule<Subrules...>(subrules...);
}

template<typename L, typename R>
auto operator+(Rule<L> const& lhs, Rule<R> const& rhs) {
    return AndRule<L,R>(lhs, rhs);
}


namespace detail {

template<typename ...L, typename R, int ...I>
auto And_splat(std::tuple<L...> const& lhs, Rule<R> const& rhs, tmpl::value_list<I...>) {
    return And(std::get<I>(lhs)..., rhs);
}

}//end namespace detail

template<typename ...L, typename R>
auto operator+(AndRule<L...> const& lhs, Rule<R> const& rhs) {
    return detail::And_splat(lhs.subrules, rhs, tmpl::arithmetic_sequence<sizeof...(L)>());
}


CPPEG_NAMESPACE_CLOSE

#endif
