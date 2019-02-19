#include "catch.hpp"
#include "cppeg.hpp"
#include <iostream>

using namespace cppeg;

TEST_CASE("Callback to set a flag") {

    std::string   s = "abba";
    InputStream<> S(s);

    bool got_it    = false;
    auto set_gotit = [&](auto const &x) { got_it = true; };
    auto a         = Char<'a'>[set_gotit];

    auto ret = a.parse(S);

    CHECK(got_it == true);

    bool ret_is_nullopt = std::is_same_v<std::decay_t<decltype(ret)>, std::optional<null_parse>>;
    CHECK(ret_is_nullopt);
    CHECK(ret.has_value());
}
