#include "Engine.h"
#include "../PartialEngine.h"

#include "Utility/Random.h"

#include "catch.hpp"

using namespace Leviathan;

class InvokeTestPartialEngine : public Test::PartialEngine<false>{
public:

    void RunInvokes(){

        {
            RecursiveLock lock(InvokeLock);
            CHECK(!InvokeQueue.empty());
        }

        ProcessInvokes();
    }
};

TEST_CASE("Invokes work", "[engine][threading]"){

    InvokeTestPartialEngine engine;

    SECTION("Basic"){

        bool invokeCalled = false;

        engine.Invoke([&](){

                invokeCalled = true;
            });
        
        engine.RunInvokes();

        CHECK(invokeCalled);
    }

    SECTION("Invoke causing invokes"){

        bool invokeCalled = false;

        engine.Invoke([&](){

                engine.Invoke([&](){

                        engine.Invoke([&](){

                                

                                invokeCalled = true;
                            });
                    });
            });
        
        engine.RunInvokes();

        CHECK(invokeCalled);        
    }
}

TEST_CASE("Invokes work from scripts", "[engine][script]"){

    CHECK(Engine::Get() == nullptr);
    
    InvokeTestPartialEngine engine;

    CHECK(Engine::Get() == &engine);

    IDFactory ids;
    ScriptExecutor exec;

	// setup the script //
	auto mod = exec.CreateNewModule("TestScript", "ScriptGenerator").lock();

    // Setup source for script //
    auto sourcecode = std::make_shared<ScriptSourceFileData>("CoreEngineTests.cpp",
        __LINE__ + 1,
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
        "}"
    );

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


TEST_CASE("Random gives values between the correct things", "[engine]"){

    Random random;

    for(int i = 0; i < 20; ++i){
        float value = random.GetNumber(0.f, 13.f);

        if(value < 0.f || value > 13.f)
            CHECK(false);
    }
}
