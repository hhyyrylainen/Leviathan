#include "../PartialEngine.h"

#include "Script/ScriptExecutor.h"
#include "Script/ScriptModule.h"
#include "Script/AccessMask.h"
#include "Addons/GameModule.h"
#include "Handlers/IDFactory.h"



#include "catch.hpp"

using namespace Leviathan;
using namespace Leviathan::Test;

int FirstTestRunSuccessFlag = 0;

TEST_CASE("Parsing AccessFlags is correct", "[script][string]"){

    SECTION("Single flag"){
        CHECK(ParseScriptAccess("Nothing") == ScriptAccess::Nothing);
        CHECK(ParseScriptAccess("DefaultEngine") == ScriptAccess::DefaultEngine);
        CHECK(ParseScriptAccess("FullFileSystem") == ScriptAccess::FullFileSystem);
    }
    
    SECTION("Multiple flag"){

        CHECK(ParseScriptAccess("FullFileSystem+DefaultEngine") == (
                static_cast<AccessFlags>(ScriptAccess::DefaultEngine) |
                static_cast<AccessFlags>(ScriptAccess::FullFileSystem)));
    }

    SECTION("Invalid throws"){

        CHECK_THROWS_AS(ParseScriptAccess("+"), InvalidArgument);
        CHECK_THROWS_AS(ParseScriptAccess("Nothi"), InvalidArgument);
        CHECK_THROWS_AS(ParseScriptAccess("Nothing+"), InvalidArgument);
        CHECK_THROWS_AS(ParseScriptAccess("Nothing+DefaultEngine+"), InvalidArgument);
    }
}

TEST_CASE("Multi file GameModule runs init", "[script][gamemodule]"){

    PartialEngine<false> engine;

    FirstTestRunSuccessFlag = 1;
    
    IDFactory ids;
    ScriptExecutor exec;

    // Filesystem required for search //
    FileSystem filesystem;
    REQUIRE(filesystem.Init(&engine.Log));

    // Register our extra variable //
    REQUIRE(exec.GetASEngine()->RegisterGlobalProperty("int FirstTestRunSuccessFlag",
            &FirstTestRunSuccessFlag) >= 0);
    
    CHECK(FirstTestRunSuccessFlag == 1);
    GameModule::pointer module;
    REQUIRE_NOTHROW(module = GameModule::MakeShared<GameModule>(
            "MultiFileTestModule", "GameModule test" + std::to_string(__LINE__)));
    REQUIRE(module);
    CHECK(module->GetRefCount() == 1);
    
    REQUIRE_NOTHROW(module->Init());
    CHECK(module->GetRefCount() == 1);
    
    // Init method changes it
    CHECK(FirstTestRunSuccessFlag != 0);

    // To this
    CHECK(FirstTestRunSuccessFlag == 1);

    // Release module //
    module->ReleaseScript();

    CHECK(module->GetRefCount() == 1);
}

TEST_CASE("GameModule ExtraAccess property works", "[script][gamemodule]"){

    PartialEngine<false> engine;
    
    IDFactory ids;
    ScriptExecutor exec;

    // Filesystem required for search //
    FileSystem filesystem;
    REQUIRE(filesystem.Init(&engine.Log));

    GameModule::pointer module;
    REQUIRE_NOTHROW(module = GameModule::MakeShared<GameModule>(
            "AccessTestModule", "GameModule test" + std::to_string(__LINE__)));
    
    REQUIRE(module);

    // We just need it to compile
    REQUIRE_NOTHROW(module->Init());

    // But we can test that the function also returns true //
    ScriptRunningSetup setup("AccessTest");
    auto result = module->ExecuteOnModule<bool>(setup, false);

    CHECK(result.Result == SCRIPT_RUN_RESULT::Success);
    CHECK(result.Value == true);

    // Release module //
    module->ReleaseScript();
}


