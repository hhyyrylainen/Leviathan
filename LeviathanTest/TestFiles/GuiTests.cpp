//! \file Tests for various supporting GUI methods. Doesn't actually
//! try any rendering or anything like that

#include "GUI/AlphaHitCache.h"

#include "GUI/VideoPlayer.h"

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

        PartialEngineWithOgre engine;
        
        AlphaHitCache cache;

        auto imageData = cache.GetDataForImageProperty("TaharezLook/ButtonMiddleNormal");

        REQUIRE(imageData);
        CHECK(imageData->HasNonZeroPixels());

        // This pixel shouldn't be fully transparent
        CHECK(imageData->GetPixel(6, 8) != 0);

        imageData = cache.GetDataForImageProperty("TaharezLook/ButtonRightNormal");

        REQUIRE(imageData);
        CHECK(imageData->HasNonZeroPixels());
        
        // This pixel should be transparent
        CHECK(imageData->GetPixel(11, 0) == 0);
    }
}


TEST_CASE("Leviathan VideoPlayer loads correctly", "[gui][video]"){
    
    // TODO: add leviathan intro video that can be attempted to be opened
    PartialEngineWithOgre engine;
    
    VideoPlayer player;

    REQUIRE(player.Play("Data/Videos/SampleVideo.mp4"));

    CHECK(player.GetDuration() == 10.334f);

    CHECK(player.GetVideoWidth() == 1920);
    CHECK(player.GetVideoHeight() == 1080);
    CHECK(player.IsStreamValid());

    player.Stop();
}
