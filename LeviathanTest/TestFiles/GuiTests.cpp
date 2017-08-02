//! \file Tests for various supporting GUI methods. Doesn't actually
//! try any rendering or anything like that

#include "GUI/AlphaHitCache.h"
#include "FileSystem.h"

#include "OgreRoot.h"
#include "OgreLogManager.h"

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

        // TODO: allow the Graphics object to be used here
        // Suppress log
        Ogre::LogManager logMgr;

        Ogre::Log* ogreLog = logMgr.createLog("Test/TestOgreLog.txt", true, false, false);

        REQUIRE(ogreLog == logMgr.getDefaultLog());
        
        Ogre::Root root("", "", "");

        Ogre::String renderSystemName = "RenderSystem_GL3Plus";

    #ifdef _DEBUG
        renderSystemName->append("_d");
    #endif // _DEBUG

    #ifndef _WIN32            
        // On platforms where rpath works plugins are in the lib subdirectory
        renderSystemName = "lib/" + renderSystemName; 
    #endif
        
        root.loadPlugin(renderSystemName);
        const auto& renderers = root.getAvailableRenderers();
        REQUIRE(renderers.size() > 0);
        REQUIRE(renderers[0]);
        root.setRenderSystem(renderers[0]);
        root.initialise(false, "", "");
        
        FileSystem filesystem;
        TestLogger reporter("Test/gui_imageset_test.txt");
        
        REQUIRE(filesystem.Init(&reporter));

        // Register resources to Ogre //
        filesystem.RegisterOGREResourceGroups(true);

        AlphaHitCache cache;

        auto imageData = cache.GetDataForImageProperty("TaharezLook/ButtonMiddleNormal");

        REQUIRE(imageData);

        // This pixel shouldn't be fully transparent
        CHECK(imageData->GetPixel(30, 30) != 0);
    }
}
