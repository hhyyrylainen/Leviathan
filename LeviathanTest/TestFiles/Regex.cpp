#include <regex>

#include "catch.hpp"

using namespace std;

TEST_CASE("Regex basic operation", "[regex][std]"){

    regex first("abd.*thing", regex_constants::ECMAScript | regex_constants::icase);

    CHECK(regex_match("abdsuper thing", first) == true);

    CHECK(regex_match("abddthingd", first) == false);

    wregex usedregex(L"matchb.+", regex_constants::ECMAScript | regex_constants::icase);

    CHECK(regex_match(L"matchb5467a", usedregex) == true);

    CHECK(regex_match(L"matchb", usedregex) == false);
}
