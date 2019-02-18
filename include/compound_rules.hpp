#ifndef CPPEG_COMPOUND_RULES
#define CPPEG_COMPOUND_RULES

#include "cppeg_common.hpp"
#include "helpers.hpp"
#include "meta.hpp"
#include "rule.hpp"

#include "tmpl.hpp"

#include <tuple>
#include <type_traits>
#include <variant>

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

        // compute the final return type
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

        // Test if all parses succeeded.
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
auto operator+(Rule<L> const &lhs, Rule<R> const &rhs) {
    return AndRule<L, R>(lhs, rhs);
}

template<typename... L, typename R>
auto operator+(AndRule<L...> const &lhs, Rule<R> const &rhs) {
    return helpers::ctor_splat<AndRule>(
        lhs.subrules, rhs, tmpl::arithmetic_sequence<sizeof...(L)>());
}

template<typename L, typename... R>
auto operator+(Rule<L> const &lhs, AndRule<R...> const &rhs) {
    return helpers::ctor_splat<AndRule>(
        lhs, rhs.subrules, tmpl::arithmetic_sequence<sizeof...(R)>());
}

//==============================================================================================
//==============================================================================================

/**
 * Returns a std::variant<std::monostate, ...>, where the other variant
 * types are the set of types returned by the sub-parsers. The first parser
 * that succeeds will be the route taken.
 */
template<typename... Subrules>
class OrRule : public Rule<OrRule<Subrules...>> {
public:
    OrRule(Rule<Subrules> const &... subrules)
        : subrules(std::forward_as_tuple(subrules.self()...)) {}

    template<typename T>
    auto parse_impl(InputStream<T> &in) {
        constexpr auto raw_subrule_return_types = tmpl::type_list<
            std::decay_t<decltype(std::declval<Subrules>().parse(in))>...>{};

        constexpr auto subrule_return_types =
            tmpl::transform(raw_subrule_return_types, [](auto &&x) {
                using xT     = tmpl::head_type_t<std::decay_t<decltype(x)>>;
                using ret_xT = meta::remove_optional_t<xT>;
                return tmpl::type_list<ret_xT>{};
            });

        constexpr auto unique_return_types =
            tmpl::make_set(subrule_return_types);

        constexpr auto return_variant_types =
            tmpl::push_front(unique_return_types, tmpl::Type<std::monostate>{});

        auto ret = tmpl::as_variant(return_variant_types);

        constexpr auto indices =
            tmpl::arithmetic_sequence<sizeof...(Subrules)>();

        bool parse_success = false;
        tmpl::for_each(indices, [&](auto x) {
            constexpr auto I = static_cast<std::size_t>(tmpl::unbox(x));
            if (parse_success) {
                return;
            }

            auto tmp       = std::get<I>(subrules).parse(in);
            using tmpT     = std::decay_t<decltype(tmp)>;
            bool I_success = false;
            if constexpr (meta::is_optional_v<tmpT>) {
                I_success = tmp.has_value();
                if (I_success) {
                    parse_success = true;
                    ret           = tmp.value();
                }
            } else if constexpr (meta::is_variant_v<tmpT>) {
                I_success = !std::holds_alternative<std::monostate>(tmp);
                if (I_success) {
                    ret = tmp;
                }
            }
        });

        return ret;
    }

    std::tuple<Subrules...> subrules;
};

template<typename... Subrules>
auto Or(Rule<Subrules> const &... subrules) {
    return OrRule<Subrules...>(subrules...);
}

template<typename L, typename R>
auto operator|(Rule<L> const &lhs, Rule<R> const &rhs) {
    return OrRule<L, R>(lhs, rhs);
}

template<typename... L, typename R>
auto operator|(OrRule<L...> const &lhs, Rule<R> const &rhs) {
    return helpers::ctor_splat<OrRule>(
        lhs.subrules, rhs, tmpl::arithmetic_sequence<sizeof...(L)>());
}

template<typename L, typename... R>
auto operator|(Rule<L> const &lhs, OrRule<R...> const &rhs) {
    return helpers::ctor_splat<OrRule>(
        lhs, rhs.subrules, tmpl::arithmetic_sequence<sizeof...(R)>());
}

//======================================================================

template<typename R, typename F>
class CallbackRule : public Rule<CallbackRule<R, F>> {

public:
    CallbackRule(const Rule<R> &subrule, F &&func)
        : subrule(subrule.self()), func(std::forward<F>(func)) {}

    template<typename T>
    auto parse_impl(InputStream<T> &in) {
        auto ret                   = subrule.parse(in);
        using callback_return_type = std::decay_t<decltype(func(ret))>;
        constexpr bool cb_returns_void =
            std::is_same_v<void, callback_return_type>;
        using return_value_type =
            std::conditional_t<cb_returns_void, null_parse,
                               callback_return_type>;

        std::optional<return_value_type> cbret;
        if (parse_success(ret)) {
            if constexpr (cb_returns_void) {
                func(ret);
                cbret = null_parse{};
            } else {
                cbret = func(ret);
            }
        }

        return cbret;
    }

private:
    R subrule;
    F func;
};

CPPEG_NAMESPACE_CLOSE

#endif
