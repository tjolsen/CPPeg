#ifndef CPPEG_INPUT_STREAM_HPP
#define CPPEG_INPUT_STREAM_HPP

#include "cppeg_common.hpp"
#include <locale>
#include <string_view>
#include <vector>

CPPEG_NAMESPACE_OPEN

// who are we kidding, we're only using chars here...
template<typename T = char>
class InputStream {

public:
    InputStream(std::basic_string_view<T> text, bool ignore_whitespace = 0)
        : m_text(text), m_ignore_whitespace(ignore_whitespace) {}

    void push() { m_pos_stack.push_back(m_pos); }

    void pop() { m_pos_stack.pop_back(); }

    void pop_reset() {
        m_pos = m_pos_stack.back();
        m_pos_stack.pop_back();
    }

    // Get the next char and advance the iterator, advancing past
    // whitespace if set to do so. m_text[m_pos] will *not*
    // equal the first returned value upon repeated calls.
    T getChar() {
        const auto N = m_text.size();
        while (m_pos < N) {
            auto C = m_text[m_pos++];
            if (!m_ignore_whitespace || !std::isspace(C)) {
                return C;
            }
        }
        return T{-1}; // probably an invalid char. can be checked for.
    }

    // Peek at the next char, but advance past whitespace
    // if set to do so. m_text[m_pos] will continue to equal
    // the first returned value upon repeated calls.
    T peekChar() {
        const auto N = m_text.size();
        while (m_pos < N) {
            auto C = m_text[m_pos];
            if (!m_ignore_whitespace || !std::isspace(C)) {
                return C;
            }
            ++m_pos;
        }
        return T{-1};
    }

    // Sometimes useful when checking has been
    // done on the other end.
    void advance_n_unchecked(int n) {
	m_pos += n;
    }
    
    // This won't work for fancier things like utf-8.
    // Fortunately, ASCII is king... (or could re-encode
    // utf-8 into utf-32)
    typename std::basic_string_view<T>::iterator get_iter() { return m_text.begin() + m_pos; }
    typename std::basic_string_view<T>::iterator end() { return m_text.end(); }

    //Distance to end. Not for utf-8
    auto distance_to_end() const { return m_text.size() - m_pos; }
    
    auto get_ignore_state() const noexcept { return m_ignore_whitespace; }
    void set_ignore_state(bool val) noexcept { m_ignore_whitespace = val; }
    auto get_pos() const noexcept { return m_pos; }

private:
    std::vector<std::size_t>  m_pos_stack;
    std::basic_string_view<T> m_text;
    std::size_t               m_pos{0};
    bool                      m_ignore_whitespace;
};

CPPEG_NAMESPACE_CLOSE

#endif
