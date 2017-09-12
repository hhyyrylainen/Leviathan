#include "Engine.h"
#include "../PartialEngine.h"

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

    exec.RunSetUp(mod.get(), &ssetup);

    CHECK(ssetup.ScriptExisted == true);

    engine.RunInvokes();
    

    // check did it exist //
    ssetup.SetEntrypoint("GetResult");
    std::shared_ptr<VariableBlock> returned = exec.RunSetUp(mod.get(), &ssetup);

    CHECK(ssetup.ScriptExisted == true);

    REQUIRE(returned);
    
    int value = *returned;

    CHECK(value == 3);
}


