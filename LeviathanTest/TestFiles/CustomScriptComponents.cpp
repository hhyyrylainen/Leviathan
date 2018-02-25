// This file has a test for registering a custom script type in a
// GameWorld and then running the system for it and getting components
// from it
#include "../PartialEngine.h"

#include "Generated/StandardWorld.h"

#include "Handlers/IDFactory.h"
#include "Script/Bindings/BindHelpers.h"
#include "Script/ScriptExecutor.h"
#include "Script/ScriptModule.h"

#include "catch.hpp"

using namespace Leviathan;
using namespace Leviathan::Test;


TEST_CASE("Script can register custom entity type and do stuff with it", "[script][entity]")
{
    PartialEngine<false> engine;

    IDFactory ids;
    ScriptExecutor exec;

    // Script needs to be valid for releasing the components
    StandardWorld world;

    // setup the script //
    auto mod = exec.CreateNewModule("TestScript", "ScriptGenerator").lock();
    CHECK(mod->AddScriptSegmentFromFile("Data/Scripts/tests/CustomScriptComponentTest.as"));

    auto module = mod->GetModule();

    REQUIRE(module != nullptr);

    ScriptRunningSetup ssetup("SetupCustomComponents");

    auto returned = exec.RunScript<bool>(mod, ssetup, static_cast<GameWorld*>(&world));

    CHECK(returned.Result == SCRIPT_RUN_RESULT::Success);
    CHECK(returned.Value == true);

    // Run the world once, and then verify //
    world.Tick(1);

    ssetup.SetEntrypoint("VerifyRunResult");

    auto returned2 = exec.RunScript<int>(mod, ssetup, static_cast<GameWorld*>(&world));

    CHECK(returned2.Result == SCRIPT_RUN_RESULT::Success);
    CHECK(returned2.Value == 141);

    REQUIRE_NOTHROW(world.Release());
}

