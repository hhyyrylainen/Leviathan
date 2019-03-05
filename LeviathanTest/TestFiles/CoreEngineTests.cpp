#include "../PartialEngine.h"
#include "Engine.h"

#include "Script/ScriptModule.h"
#include "Utility/Random.h"

#include "catch.hpp"

using namespace Leviathan;
using namespace Leviathan::Test;


class InvokeTestPartialEngine : public Test::PartialEngine<false> {
public:
    void RunInvokes()
    {

        {
            RecursiveLock lock(InvokeLock);
            CHECK(!InvokeQueue.empty());
        }

        ProcessInvokes();
    }
};

TEST_CASE("Invokes work", "[engine][threading]")
{
    InvokeTestPartialEngine engine;

    SECTION("Basic")
    {

        bool invokeCalled = false;

        engine.Invoke([&]() { invokeCalled = true; });

        engine.RunInvokes();

        CHECK(invokeCalled);
    }

    SECTION("Invoke causing invokes")
    {

        bool invokeCalled = false;

        engine.Invoke([&]() {
            engine.Invoke([&]() { engine.Invoke([&]() { invokeCalled = true; }); });
        });

        engine.RunInvokes();

        CHECK(invokeCalled);
    }
}

TEST_CASE("Invokes work from scripts", "[engine][script]")
{
    CHECK(Engine::Get() == nullptr);

    InvokeTestPartialEngine engine;

    CHECK(Engine::Get() == &engine);

    IDFactory ids;
    ScriptExecutor exec;

    // setup the script //
    auto mod = exec.CreateNewModule("TestScript", "ScriptGenerator").lock();

    // Setup source for script //
    auto sourcecode =
        std::make_shared<ScriptSourceFileData>("CoreEngineTests.cpp", __LINE__ + 1,
            "int InvokeCounter = 0;\n"
            "\n"
            "void InvokeCalled(){\n"
            "\n"
            "    InvokeCounter++;\n"
            "\n"
            "    if(InvokeCounter < 3)\n"
            "        GetEngine().Invoke(@InvokeCalled);\n"
            "}\n"
            "\n"
            "void TestInvoke(){\n"
            "\n"
            "    GetEngine().Invoke(@InvokeCalled);\n"
            "}\n"
            "\n"
            "int GetResult(){\n"
            "\n"
            "    return InvokeCounter;\n"
            "}");

    mod->AddScriptSegment(sourcecode);

    auto module = mod->GetModule();

    REQUIRE(module != nullptr);

    ScriptRunningSetup ssetup;
    ssetup.SetEntrypoint("TestInvoke").SetUseFullDeclaration(false);

    auto result = exec.RunScript<void>(mod, ssetup);

    CHECK(ssetup.ScriptExisted == true);
    CHECK(result.Result == SCRIPT_RUN_RESULT::Success);

    engine.RunInvokes();


    // check did it exist //
    ssetup.SetEntrypoint("GetResult");
    auto result2 = exec.RunScript<int>(mod, ssetup);

    CHECK(ssetup.ScriptExisted == true);
    REQUIRE(result2.Result == SCRIPT_RUN_RESULT::Success);

    CHECK(result2.Value == 3);
}

static Random testRandom;

Random* GetTestRandom()
{
    return &testRandom;
}

TEST_CASE("Random gives values between the correct things", "[engine][random]")
{
    SECTION("0.f, 13.f")
    {
        Random random;

        for(int i = 0; i < 20; ++i) {
            float value = random.GetNumber(0.f, 13.f);

            if(value < 0.f || value > 13.f)
                CHECK(false);
        }
    }

    SECTION("0, 1 gives both 0 and 1 at some point")
    {
        SECTION("C++ side")
        {
            Random random;

            bool zero = false;
            bool one = false;

            for(int i = 0; i < std::numeric_limits<int>::max(); ++i) {

                int val = random.GetNumber(0, 1);
                if(val == 0)
                    zero = true;
                if(val == 1)
                    one = true;
                if(zero && one)
                    break;
            }

            CHECK(zero);
            CHECK(one);
        }

        SECTION("AngelScript")
        {
            PartialEngine<false> engine;

            IDFactory ids;
            ScriptExecutor exec;

            REQUIRE(exec.GetASEngine()->RegisterGlobalFunction(
                        "Random& GetRandom()", asFUNCTION(GetTestRandom), asCALL_CDECL) >= 0);

            // setup the script //
            auto mod = exec.CreateNewModule("TestScript", "ScriptGenerator").lock();

            // Setup source for script //
            auto sourcecode =
                std::make_shared<ScriptSourceFileData>("CoreEngineTests.cpp", __LINE__ + 1,
                    "bool TestFunction(){\n"
                    "bool zero = false;\n"
                    "bool one = false;\n"
                    "for(int i = 0; i < 2147483647; ++i) {\n"
                    "    int val = GetRandom().GetNumber(0, 1);\n"
                    "    if(val == 0)\n"
                    "        zero = true;\n"
                    "    if(val == 1)\n"
                    "        one = true;\n"
                    "    if(zero && one)\n"
                    "        break;\n"
                    "}\n"
                    "return zero == true && one == true;\n"
                    "}");

            mod->AddScriptSegment(sourcecode);

            auto module = mod->GetModule();

            REQUIRE(module != nullptr);


            ScriptRunningSetup ssetup("TestFunction");

            auto returned = exec.RunScript<bool>(mod, ssetup);

            REQUIRE(returned.Result == SCRIPT_RUN_RESULT::Success);
            CHECK(returned.Value == true);
        }
    }
}
