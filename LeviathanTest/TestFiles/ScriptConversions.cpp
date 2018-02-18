#include "../PartialEngine.h"

#include "../OtherTestHelpers.h"

#include "Handlers/IDFactory.h"
#include "Script/Bindings/BindHelpers.h"
#include "Script/ScriptConversionHelpers.h"
#include "Script/ScriptExecutor.h"
#include "Script/ScriptModule.h"

#include "catch.hpp"

using namespace Leviathan;
using namespace Leviathan::Test;

std::vector<std::string> Test1ExpectedData = {"line1", "second line", "third"};

auto GetTestDataArray1()
{
    // Needs to be called from a script
    asIScriptEngine* engine = asGetActiveContext()->GetEngine();
    REQUIRE(engine);
    return ConvertVectorToASArray(Test1ExpectedData, engine);
}


TEST_CASE("Returning script arrays to scripts works", "[script]")
{
    // This gets leaked quite easily if this test fails, but we want
    // to expose this as global property so this has to be raw pointer
    // to work without a second wrapper
    RequirePassSequence* pass = nullptr;

    PartialEngine<false> engine;

    IDFactory ids;
    ScriptExecutor exec;

    REQUIRE(RequirePassSequence::Register(exec.GetASEngine()));

    REQUIRE(exec.GetASEngine()->RegisterGlobalProperty(
                "RequirePassSequence@ sequence", &pass) >= 0);

    REQUIRE(exec.GetASEngine()->RegisterGlobalFunction("array<string>@ GetTestDataArray1()",
                asFUNCTION(GetTestDataArray1), asCALL_CDECL) >= 0);

    // setup the script //
    auto mod = exec.CreateNewModule("TestScript", "ScriptGenerator").lock();

    auto sourcecode =
        std::make_shared<ScriptSourceFileData>("ScriptConversions.cpp", __LINE__ + 1,
            "void TestFunction(){\n"
            "array<string> data = GetTestDataArray1();\n"
            "for(uint i = 0; i < data.length(); ++i){\n"
            "sequence.Provide(data[i]);\n"
            "}\n"
            "}");

    mod->AddScriptSegment(sourcecode);

    auto module = mod->GetModule();

    REQUIRE(module != nullptr);

    ScriptRunningSetup ssetup("TestFunction");

    pass = new RequirePassSequence(Test1ExpectedData);

    auto returned = exec.RunScript<void>(mod, ssetup);

    CHECK(returned.Result == SCRIPT_RUN_RESULT::Success);

    CHECK(pass->AllProvided());
    pass->Release();
}
