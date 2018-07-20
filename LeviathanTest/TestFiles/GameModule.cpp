#include "../PartialEngine.h"

#include "Addons/GameModule.h"
#include "Addons/GameModuleLoader.h"
#include "Exceptions.h"
#include "Handlers/IDFactory.h"
#include "Script/AccessMask.h"
#include "Script/ScriptExecutor.h"
#include "Script/ScriptModule.h"


#include "catch.hpp"

using namespace Leviathan;
using namespace Leviathan::Test;

int FirstTestRunSuccessFlag = 0;

TEST_CASE("Parsing AccessFlags is correct", "[script][string]")
{
    SECTION("Single flag")
    {
        CHECK(ParseScriptAccess("Nothing") == ScriptAccess::Nothing);
        CHECK(ParseScriptAccess("DefaultEngine") == ScriptAccess::DefaultEngine);
        CHECK(ParseScriptAccess("FullFileSystem") == ScriptAccess::FullFileSystem);
    }

    SECTION("Multiple flag")
    {

        CHECK(ParseScriptAccess("FullFileSystem+DefaultEngine") ==
              (static_cast<AccessFlags>(ScriptAccess::DefaultEngine) |
                  static_cast<AccessFlags>(ScriptAccess::FullFileSystem)));
    }

    SECTION("Invalid throws")
    {

        CHECK_THROWS_AS(ParseScriptAccess("+"), InvalidArgument);
        CHECK_THROWS_AS(ParseScriptAccess("Nothi"), InvalidArgument);
        CHECK_THROWS_AS(ParseScriptAccess("Nothing+"), InvalidArgument);
        CHECK_THROWS_AS(ParseScriptAccess("Nothing+DefaultEngine+"), InvalidArgument);
    }
}

TEST_CASE("Multi file GameModule runs init", "[script][gamemodule]")
{
    PartialEngine<false> engine;

    FirstTestRunSuccessFlag = 1;

    IDFactory ids;
    ScriptExecutor exec;

    // Filesystem required for search //
    FileSystem filesystem;
    REQUIRE(filesystem.Init(&engine.Log));
    GameModuleLoader loader;
    loader.Init();

    // Register our extra variable //
    REQUIRE(exec.GetASEngine()->RegisterGlobalProperty(
                "int FirstTestRunSuccessFlag", &FirstTestRunSuccessFlag) >= 0);

    CHECK(FirstTestRunSuccessFlag == 1);
    GameModule::pointer module;
    REQUIRE_NOTHROW(module = loader.Load("MultiFileTestModule",
                        ("GameModule test" + std::to_string(__LINE__)).c_str()));

    // Init is called by the loader

    REQUIRE(module);
    CHECK(module->GetRefCount() == 1);

    // Init method changes it
    CHECK(FirstTestRunSuccessFlag != 0);

    // To this
    CHECK(FirstTestRunSuccessFlag == 1);

    // Release module //
    module->ReleaseScript();

    CHECK(module->GetRefCount() == 1);
}

TEST_CASE("GameModule ExtraAccess property works", "[script][gamemodule]")
{
    PartialEngine<false> engine;

    IDFactory ids;
    ScriptExecutor exec;

    // Filesystem required for search //
    FileSystem filesystem;
    REQUIRE(filesystem.Init(&engine.Log));
    GameModuleLoader loader;
    loader.Init();

    GameModule::pointer module;
    // We just need it to compile
    REQUIRE_NOTHROW(module = loader.Load("AccessTestModule",
                        ("GameModule test" + std::to_string(__LINE__)).c_str()));

    REQUIRE(module);

    // But we can test that the function also returns true //
    ScriptRunningSetup setup("AccessTest");
    auto result = module->ExecuteOnModule<bool>(setup, false);

    CHECK(result.Result == SCRIPT_RUN_RESULT::Success);
    CHECK(result.Value == true);

    // Release module //
    module->ReleaseScript();
}


TEST_CASE("GameModule script doesn't have all access set", "[script][gamemodule]")
{
    PartialEngine<false> engine;
    // Overwrite the logger in engine
    TestLogRequireError requireError;
    requireError.WarningsCountAsErrors = true;

    REQUIRE(Logger::Get() == &requireError);

    IDFactory ids;
    ScriptExecutor exec;

    // Filesystem required for search //
    FileSystem filesystem;
    REQUIRE(filesystem.Init(&engine.Log));
    GameModuleLoader loader;
    loader.Init();

    GameModule::pointer module;
    // It needs to fail to load
    REQUIRE_THROWS_AS(module = loader.Load("AccessTestFailModule",
                         ("GameModule test" + std::to_string(__LINE__)).c_str()),
        NotFound);

    // It needs to have failed
    REQUIRE(!module);

    REQUIRE(Logger::Get() == &requireError);
}


TEST_CASE("GameModule publicdefinitions and imports work", "[script][gamemodule]")
{
    PartialEngine<false> engine;

    IDFactory ids;
    ScriptExecutor exec;

    // Filesystem required for search //
    FileSystem filesystem;
    REQUIRE(filesystem.Init(&engine.Log));
    GameModuleLoader loader;
    loader.Init();

    GameModule::pointer module;
    REQUIRE_NOTHROW(module = loader.Load("SimpleImportModule",
                        ("GameModule test" + std::to_string(__LINE__)).c_str()));

    REQUIRE(module);

    // Test that it runs. This is sort of extra as if the module compiled the import most
    // likely succeeded
    ScriptRunningSetup setup("TestImportedFunction");
    auto result = module->ExecuteOnModule<int>(setup, false, 13);

    CHECK(result.Result == SCRIPT_RUN_RESULT::Success);
    CHECK(result.Value == 13 * 13);

    module->ReleaseScript();
}
