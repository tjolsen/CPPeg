#ifndef CPPEG_RULE_HPP
#define CPPEG_RULE_HPP

#include "cppeg_common.hpp"

CPPEG_NAMESPACE_OPEN

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

	//pass the call through to the 'real' parser rule.
	auto ret = self().parse_impl(inputStream);

        if (ret.has_value()) {
            inputStream.pop();
        } else {
            inputStream.pop_reset();
        }

        return ret;
    }

    // operators

};

CPPEG_NAMESPACE_CLOSE

#endif
