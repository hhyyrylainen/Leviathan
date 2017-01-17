
//! \file Testing for functions in ExtraAlgorithms

#include "Common/ExtraAlgorithms.h"

#include "catch.hpp"

using namespace Leviathan;
using namespace std;

TEST_CASE("ExtraAlgorithms::FindRemovedElements", "[algorithm]"){

    SECTION("Basic strings"){

        std::vector<std::string> original {"first", "second", "more stuff", "final thing"};
        std::vector<std::string> changed {"second", "final thing", "more"};

        std::vector<std::string> added;
        std::vector<std::string> removed;
        
        FindRemovedElements(original, changed, added, removed);

        REQUIRE(added.size() == 1);
        REQUIRE(removed.size() == 2);

        CHECK(added[0] == "more");
        CHECK(removed[0] == "first");
        CHECK(removed[1] == "more stuff");
    }


    SECTION("Numbers"){

        std::vector<int32_t> original {2, 4, 6, 8, 11};
        std::vector<int32_t> changed {2, 3, 5, 6, 8};

        std::vector<int32_t> added;
        std::vector<int32_t> removed;
        
        FindRemovedElements(original, changed, added, removed);

        REQUIRE(added.size() == 2);
        REQUIRE(removed.size() == 2);

        CHECK(added[0] == 3);
        CHECK(added[1] == 5);
        CHECK(removed[0] == 4);
        CHECK(removed[1] == 11);
    }
}
