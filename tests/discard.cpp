#include "cppeg.hpp"
#include "catch.hpp"

using namespace cppeg;


TEST_CASE("Test discard rule") {

    std::string s = "abba";
    InputStream<> S(s);


    auto parser = ~(Char<'a'> + "bb"_L + Char<'a'>);

    auto ret = parser.parse(S);
    using ret_t = std::decay_t<decltype(ret)>;
    bool correct_type = std::is_same_v<ret_t, std::optional<null_parse>>;
    CHECK(parse_success(ret));
    CHECK(correct_type);
}


TEST_CASE("AndRule with discard in middle") {

    std::string s = "abba";
    InputStream<> S(s);


    auto parser = Char<'a'> + ~("bb"_L) + Char<'a'>;

    auto ret = parser.parse(S);
    using ret_t = std::decay_t<decltype(ret)>;
    bool correct_type = std::is_same_v<ret_t, std::optional<std::tuple<char,char>>>;
    auto correct_answer = std::tuple<char,char>('a', 'a');
    CHECK(parse_success(ret));
    CHECK(correct_type);
    CHECK(ret.value() == correct_answer);
}
