#ifndef CPPEG_COMMON_HPP
#define CPPEG_COMMON_HPP

#define CPPEG_NAMESPACE_OPEN namespace cppeg {
#define CPPEG_NAMESPACE_CLOSE }


CPPEG_NAMESPACE_OPEN

/**
 * Special type that indicates that the parser
 * should be interpreted as having *no* return
 * type. However, "void" is shitty and not a type
 * that plays well with metaprogramming facilities.
 *
 * This type will be ignored in sequences of parse
 * results (e.g. AndRule). This can make it useful
 * as a way to "discard" a result. One may wish to
 * do this if (eg) a parser has side-effects (via
 * CallbackRule) or if you simply want to discard
 * delimiters, whitespace, etc.
 */
struct null_parse{};


CPPEG_NAMESPACE_CLOSE


#endif
