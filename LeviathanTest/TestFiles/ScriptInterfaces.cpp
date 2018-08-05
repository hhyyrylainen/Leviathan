#include "../PartialEngine.h"

#include "Script/ScriptExecutor.h"
#include "Script/ScriptModule.h"


#include "catch.hpp"

using namespace Leviathan;
using namespace Leviathan::Test;


TEST_CASE("Test script EventListener", "[script][event]")
{
    PartialEngine<false> engine;

    IDFactory ids;
    ScriptExecutor exec;

    // setup the script //
    auto mod = exec.CreateNewModule("TestScript", "ScriptGenerator").lock();
    REQUIRE(mod->AddScriptSegmentFromFile("Data/Scripts/tests/TestEventListener.as"));

    auto module = mod->GetModule();

    REQUIRE(module != nullptr);

    ScriptRunningSetup ssetup("StartListen");

    ScriptRunResult<bool> result =
        exec.RunScript<bool>(mod, ssetup, std::string("TestEvent1"));

    REQUIRE(result.Result == SCRIPT_RUN_RESULT::Success);
    CHECK(result.Value == true);

    GenericEvent::pointer event = GenericEvent::MakeShared<GenericEvent>("TestEvent1");
    event->GetVariables()->Add(std::make_shared<NamedVariableList>("val", new IntBlock(2)));

    engine.GetEventHandler()->CallEvent(event);

    ssetup.SetEntrypoint("CheckListen");

    const auto result2 = exec.RunScript<int>(mod, ssetup);

    REQUIRE(result2.Result == SCRIPT_RUN_RESULT::Success);
    CHECK(result2.Value == 8);

    mod->DeleteThisModule();
}
