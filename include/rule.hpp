#ifndef CPPEG_RULE_HPP
#define CPPEG_RULE_HPP

#include "cppeg_common.hpp"
#include "meta.hpp"
#include "assertions.hpp"
#include <optional>
#include <variant>


CPPEG_NAMESPACE_OPEN

template<typename T>
bool parse_success(T const &x) {
    if constexpr (meta::is_optional_v<T>) {
        return x.has_value();
    } else if constexpr (meta::is_variant_v<T>) {
        return !std::holds_alternative<std::monostate>(x);
    } else {
	debug_assert(false, "Unknown parse return type.");
	return false;
    }
}

template<typename R>
struct Rule {

    // We're using Expression Templates...
    R &      self() { return static_cast<R &>(*this); }
    R const &self() const { return static_cast<R const &>(*this); }

    // The actual workhorse function. Injects push/pop calls as appropriate.
    // Returns a std::optional<...>, which is received from the
    // self().parse_impl(...) call.
    template<typename T>
    auto parse(InputStream<T> &inputStream) {
        inputStream.push(); // save current spot in stream

        // pass the call through to the 'real' parser rule.
        auto ret = self().parse_impl(inputStream);

        if (parse_success(ret)) {
            inputStream.pop();
        } else {
            inputStream.pop_reset();
        }

        return ret;
    }

};

CPPEG_NAMESPACE_CLOSE

#endif
