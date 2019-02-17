#include "cppeg.hpp"
#include "catch.hpp"
using namespace cppeg;

TEST_CASE("OrRule test: identical types") {

    std::string s = "abba";
    InputStream<> S(s);
    
    auto ab = Char<'a'> | Char<'b'>;
    auto cab = Char<'c'> | Char<'a'> | Char<'b'>;

    auto abret = ab.parse(S);
    auto cabret = cab.parse(S);

    CHECK(std::holds_alternative<char>(abret));
    CHECK(std::holds_alternative<char>(cabret));

    CHECK(std::get<char>(abret) == 'a');
    CHECK(std::get<char>(cabret) == 'b');
}

TEST_CASE("OrRule test: Two different types") {

    std::string s = "abba";
    InputStream<> S(s);

    auto abb = Char<'a'> | "bb"_L;

    auto ret1 = abb.parse(S);
    auto ret2 = abb.parse(S);

    CHECK(std::holds_alternative<char>(ret1));
    CHECK(std::holds_alternative<std::string>(ret2));

    CHECK(std::get<char>(ret1) == 'a');
    CHECK(std::get<std::string>(ret2) == "bb");
}
