#include "cppeg.hpp"
#include "catch.hpp"
#include <iostream>

using namespace cppeg;

TEST_CASE("And rule: 2 chars") {

    std::string s = "abba";
    InputStream<> S(s);

    auto ab_parser = And( Char<'a'>, Char<'b'> );
    
    auto ret = ab_parser.parse(S);
    CHECK(ret.has_value() );

    auto answer_ab = std::tuple<char,char>('a', 'b');
    CHECK(ret.value() == answer_ab);


    std::cout << "ret type: " << helpers::type_name<decltype(ret)>() << "\n";
}

TEST_CASE("And rule: 3 chars") {

    std::string s = "abba";
    InputStream<> S(s);
    InputStream<> S2(s);

    auto abb_parser = And( Char<'a'>, Char<'b'> , Char<'b'>);
    auto abc_parser = And( Char<'a'>, Char<'b'> , Char<'c'>);

    auto ret = abb_parser.parse(S);
    auto answer_abb = std::tuple<char,char,char>('a', 'b', 'b');
    CHECK(ret.has_value() );    
    CHECK(ret.value() == answer_abb);


    auto ret2 = abc_parser.parse(S2);
    CHECK(ret2.has_value() == false);
}

TEST_CASE("And rule: heterogeneous") {

    std::string s = "abba";
    InputStream<> S(s);
    InputStream<> S2(s);

    auto abba_parser = And( Char<'a'>, "bb"_L , Char<'a'>);

    auto ret = abba_parser.parse(S);
    auto answer_abba = std::tuple<char,std::string,char>('a', "bb", 'a');
    CHECK(ret.has_value() );
    CHECK(ret.value() == answer_abba);
    std::cout << "ret type: " << helpers::type_name<decltype(ret)>() << "\n";
}


TEST_CASE("And rule: heterogeneous with operator+") {

    std::string s = "abba";
    InputStream<> S(s);
    InputStream<> S2(s);

    auto abba_parser = Char<'a'> + "bb"_L + Char<'a'>;

    auto ret = abba_parser.parse(S);
    auto answer_abba = std::tuple<char,std::string,char>('a', "bb", 'a');
    CHECK(ret.has_value() );
    CHECK(ret.value() == answer_abba);
    std::cout << "ret type: " << helpers::type_name<decltype(ret)>() << "\n";
}
