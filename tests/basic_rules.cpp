#include "cppeg.hpp"
#include "catch.hpp"
using namespace cppeg;

TEST_CASE("Basic Rule Tests") {
    std::string s = "This is a test string";
    InputStream<> S(s);
    
    auto parser = "This"_L;
    auto ret = parser.parse(S);
    CHECK(ret.has_value());
    CHECK(*ret == "This");

    auto not_good = "Asdf"_L;
    auto cur_pos = S.get_pos();
    auto ret2 = not_good.parse(S);
    CHECK(ret2.has_value() == false);

    //failure does not advance stream
    CHECK(S.get_pos() == cur_pos);
}
