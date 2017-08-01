//! \file Tests for various supporting GUI methods. Doesn't actually
//! try any rendering or anything like that

#include "GUI/AlphaHitCache.h"
#include "FileSystem.h"

#include "../PartialEngine.h"

#include "catch.hpp"

using namespace Leviathan;
using namespace Leviathan::Test;
using namespace Leviathan::GUI;


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

        FileSystem filesystem;
        TestLogger reporter("Test/gui_imageset_test.txt");
        
        REQUIRE(filesystem.Init(&reporter));

        AlphaHitCache cache;

        auto imageData = cache.GetDataForImageProperty("TaharezLook/ButtonMiddleNormal");

        REQUIRE(imageData);

        // This pixel shouldn't be fully transparent
        CHECK(imageData->GetPixel(30, 30) != 0);
    }
}
