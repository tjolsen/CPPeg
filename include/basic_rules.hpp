#ifndef CPPEG_BASIC_RULES
#define CPPEG_BASIC_RULES

#include "cppeg_common.hpp"
#include "rule.hpp"
#include <optional>
#include <cmath>

#include <iostream> //for debugging

CPPEG_NAMESPACE_OPEN

template<char C>
struct CharRule : public Rule<CharRule<C>> {

    template<typename T>
    auto parse_impl(InputStream<T> &in) {
        std::optional<char> ret;
        auto c = in.getChar();
        if (c == C) {
            ret = C;
        }
        return ret;
    }
};

template<char C>
constexpr CharRule<C> Char = CharRule<C>{};


template<char First, char Last>
struct CharRng : public Rule<CharRng<First, Last>> {
    template<typename T>
    auto parse_impl(InputStream<T> &in) {
        auto c = in.getChar();
        std::optional<char> ret;
        if (c >= First && c <= Last) {
            ret = c;
        }
        return ret;
    }
};


struct Literal : public Rule<Literal> {
    Literal(std::string const& s) : m_literal(s) {}
    Literal(std::string &&s) : m_literal(std::forward<std::string>(s)) {}

    template<typename T>
    auto parse_impl(InputStream<T> &in) {
	auto N = m_literal.size();
	std::optional<std::string> ret;

	auto c = in.peekChar();

	auto stream_iter = in.get_iter();
	auto stream_end = in.end();
	auto dist_to_end = in.distance_to_end();

	auto in_stream = std::basic_string_view<T>(stream_iter, std::min(N, dist_to_end));
	if(m_literal == in_stream) {
	    ret = m_literal;
	    in.advance_n_unchecked(in_stream.size());
	}
	
	return ret;
    }

private:
    std::string m_literal;
};

//Convenience operator to turn a string literal into a cppeg::Literal
inline Literal operator "" _L(const char* str, std::size_t N) {
    return Literal(std::string(str,N));
}


template<char ...Cs>
struct AnyChar : public Rule<AnyChar<Cs...>> {
    template<typename T>
    auto parse_impl(InputStream<T> &in) {
	auto c = in.getNext();
	std::optional<char> ret;
	if( ( (Cs==c) || ...) ) {
	    ret = c;
	}
	return ret;
    }
};

CPPEG_NAMESPACE_CLOSE

#endif
