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

        // compute the return types, pre-filtering
        constexpr auto parse_result_type_list =
            tmpl::transform(optional_parse_result_types{}, [](auto &&x) {
                using xT = tmpl::head_type_t<decltype(x)>;
                return tmpl::type_list<meta::remove_optional_t<xT>>{};
            });

        // compute the list of return types, filtering out null_parse types.
        // must keep track of the original position in the pre-filtering
        // list, in order to grab the result from the correct parser.
        constexpr auto parse_result_type_indices =
            tmpl::zip(parse_result_type_list, indices);
        constexpr auto filtered_list =
            tmpl::select_if(parse_result_type_indices, [](auto &&x) constexpr {
                return tmpl::unbox(x).type() != tmpl::type_list<null_parse>{};
            });

        constexpr auto filtered_types =
            tmpl::transform(filtered_list, [](auto &&x) {
                constexpr auto X = tmpl::unbox(std::decay_t<decltype(x)>{});
                return X.type();
            });

        using return_tuple_type =
            std::decay_t<decltype(tmpl::as_tuple(filtered_types))>;

        std::optional<return_tuple_type> ret;

        // make a tuple of optionals to hold intermediate parses
        auto tmp_ret = tmpl::as_tuple(optional_parse_result_types{});

        bool parse_success_sofar = true;
        tmpl::for_each(tmpl::zip(types, indices), [&](auto &&tv_list) {
            if (!parse_success_sofar) {
                return;
            }

            auto tv    = tmpl::unbox(tv_list);
            using type = typename std::decay_t<decltype(tv)>::type_t;
            constexpr std::size_t I = tv.value();

            std::get<I>(tmp_ret) = std::get<I>(subrules).parse(in);
            parse_success_sofar =
                parse_success_sofar && parse_success(std::get<I>(tmp_ret));
        });

        if (parse_success_sofar) {
            return_tuple_type parse_results;
            constexpr auto    out_indices =
                tmpl::arithmetic_sequence<filtered_types.size()>();
            tmpl::for_each(tmpl::zip(filtered_list, out_indices), [&](auto x) {
                constexpr auto X    = tmpl::unbox(std::decay_t<decltype(x)>{});
                constexpr auto outI = static_cast<std::size_t>(X.value());
                constexpr auto filtered_tv = X.type();
                constexpr auto parserI =
                    static_cast<std::size_t>(tmpl::unbox(filtered_tv).value());
                std::get<outI>(parse_results) =
                    std::move(std::get<parserI>(tmp_ret).value());
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

//======================================================================

template<typename R>
class DiscardRule : public Rule<DiscardRule<R>> {

public:
    DiscardRule(const Rule<R> &r) : subrule(r.self()) {}

    template<typename T>
    auto parse_impl(InputStream<T> &in) {

        std::optional<null_parse> ret;
        auto                      sub_ret = subrule.parse(in);

        if (parse_success(sub_ret)) {
            ret = null_parse{};
        }

        return ret;
    }

private:
    R subrule;
};

template<typename R>
auto operator~(const Rule<R> &r) {
    return DiscardRule<R>(r);
}

CPPEG_NAMESPACE_CLOSE

#endif
