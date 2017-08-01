//! \file Tests for various supporting GUI methods. Doesn't actually
//! try any rendering or anything like that

#include "GUI/AlphaHitCache.h"

#include "catch.hpp"

using namespace Leviathan;
using namespace GUI;


TEST_CASE("CEGUI Image property is correctly parsed", "[gui]"){

    SECTION("The text is properly split"){

        SECTION("TaharezLook/ButtonMiddleNormal"){

            std::string schema, name;
            std::tie(schema, name) = AlphaHitCache::ParseImageProperty(
                "TaharezLook/ButtonMiddleNormal");

            CHECK(schema == "TaharezLook");
            CHECK(name == "ButtonMiddleNormal");
        }

        SECTION("ThriveGeneric/MenuNormal"){
            
            std::string schema, name;
            std::tie(schema, name) = AlphaHitCache::ParseImageProperty(
                "ThriveGeneric/MenuNormal");

            CHECK(schema == "ThriveGeneric");
            CHECK(name == "MenuNormal");
        }        
    }

    SECTION("Taharezlook imageset can be properly read"){

        
    }
}
