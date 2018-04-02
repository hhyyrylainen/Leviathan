#include "Input/Key.h"

#include "catch.hpp"
#include "../PartialEngine.h"

using namespace Leviathan;
using namespace Test;

TEST_CASE("Common key names parsing", "[string][input]"){
    
    TestLogger log;
    
    SECTION("ESCAPE"){

        const auto key = GKey::GenerateKeyFromString("ESCAPE");
        // 27 == ESC from asciitable
        CHECK(key.GetCharacter() == 27);
    }
}

TEST_CASE("Invalid key name gives an error", "[string][input]"){

    // Needs to print an error. In the future this might throw //
    TestLogRequireError log;

    const auto key = GKey::GenerateKeyFromString("not a key");

    CHECK(key.GetCharacter() == 0);
}


