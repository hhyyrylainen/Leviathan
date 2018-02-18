#include "../PartialEngine.h"

#include "../OtherTestHelpers.h"

#include "Handlers/IDFactory.h"
#include "Script/Bindings/BindHelpers.h"
#include "Script/ScriptConversionHelpers.h"
#include "Script/ScriptExecutor.h"
#include "Script/ScriptModule.h"

#include <boost/range/adaptor/map.hpp>

#include "catch.hpp"

using namespace Leviathan;
using namespace Leviathan::Test;

std::vector<std::string> Test1ExpectedData = {"line1", "second line", "third"};

std::vector<std::string> Test2ExpectedData = {"1", "4", "6", "9"};

auto GetTestDataArray1()
{
    // Needs to be called from a script
    asIScriptEngine* engine = asGetActiveContext()->GetEngine();
    REQUIRE(engine);
    return ConvertVectorToASArray(Test1ExpectedData, engine);
}

auto GetTestDataArray2()
{
    // Needs to be called from a script
    asIScriptEngine* engine = asGetActiveContext()->GetEngine();
    REQUIRE(engine);

    std::map<unsigned int, std::string> testData = {
        {1, "stuff"}, {9, "a"}, {4, "b"}, {6, "c"}};

    return ConvertIteratorToASArray(std::begin(testData | boost::adaptors::map_keys),
        std::end(testData | boost::adaptors::map_keys), engine);
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

    REQUIRE(exec.GetASEngine()->RegisterGlobalFunction("array<uint>@ GetTestDataArray2()",
                asFUNCTION(GetTestDataArray2), asCALL_CDECL) >= 0);

    // setup the script //
    auto mod = exec.CreateNewModule("TestScript", "ScriptGenerator").lock();

    auto sourcecode =
        std::make_shared<ScriptSourceFileData>("ScriptConversions.cpp", __LINE__ + 1,
            "void TestFunction(){\n"
            "array<string> data = GetTestDataArray1();\n"
            "for(uint i = 0; i < data.length(); ++i){\n"
            "sequence.Provide(data[i]);\n"
            "}\n"
            "}\n"
            "void TestFunction2(){\n"
            "array<uint> data = GetTestDataArray2();\n"
            "data.sortAsc();\n"
            "for(uint i = 0; i < data.length(); ++i){\n"
            "sequence.Provide(formatUInt(data[i]));\n"
            "}\n"
            "}");

    mod->AddScriptSegment(sourcecode);

    auto module = mod->GetModule();

    REQUIRE(module != nullptr);

    SECTION("From std::vector<std::string>")
    {
        ScriptRunningSetup ssetup("TestFunction");

        pass = new RequirePassSequence(Test1ExpectedData);

        auto returned = exec.RunScript<void>(mod, ssetup);

        CHECK(returned.Result == SCRIPT_RUN_RESULT::Success);
    }

    SECTION("From std::map<unsigned int, std::string> just keys")
    {
        ScriptRunningSetup ssetup("TestFunction2");

        pass = new RequirePassSequence(Test2ExpectedData);

        auto returned = exec.RunScript<void>(mod, ssetup);

        CHECK(returned.Result == SCRIPT_RUN_RESULT::Success);
    }

    CHECK(pass->AllProvided());
    pass->Release();
}
