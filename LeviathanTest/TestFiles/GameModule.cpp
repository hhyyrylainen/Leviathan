#include "../PartialEngine.h"

#include "Script/ScriptExecutor.h"
#include "Script/ScriptModule.h"
#include "Addons/GameModule.h"
#include "Handlers/IDFactory.h"

#include "catch.hpp"

using namespace Leviathan;
using namespace Leviathan::Test;

int FirstTestRunSuccessFlag = 0;

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
