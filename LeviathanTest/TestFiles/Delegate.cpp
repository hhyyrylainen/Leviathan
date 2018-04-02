#include "../PartialEngine.h"

#include "Events/DelegateSlot.h"
#include "Script/ScriptExecutor.h"
#include "Script/ScriptModule.h"

#include "catch.hpp"

using namespace Leviathan;

TEST_CASE("Lambda delegates work", "[delegate]"){

    Delegate slot;

    bool lambda1Called = false;
    bool lambda2Called = false;

    slot.Register(LambdaDelegateSlot::MakeShared<LambdaDelegateSlot>(
                [&](const NamedVars::pointer&) -> void {

                    lambda1Called = true;
                    
                })); 

    slot.Register(LambdaDelegateSlot::MakeShared<LambdaDelegateSlot>(
                [&](const NamedVars::pointer&) -> void {

                    lambda2Called = true;
                    
                })); 
    
    CHECK(!lambda1Called);
    CHECK(!lambda2Called);

    slot.Call(NamedVars::MakeShared<NamedVars>());

    CHECK(lambda1Called);
    CHECK(lambda2Called);    
}


TEST_CASE("Script can register and call delegate", "[delegate][script]"){

    Test::PartialEngine<false> engine;
    
    IDFactory ids;
    ScriptExecutor exec;

	// setup the script //
	auto mod = exec.CreateNewModule("TestScript", "ScriptGenerator").lock();

    // Setup source for script //
    auto sourcecode = std::make_shared<ScriptSourceFileData>("Delegate.cpp", __LINE__ + 1,
        "int ReadValue = 0;\n"
        "\n"
        "void Callback(NamedVars@ variables){\n"
        "\n"
        "    auto value = variables.GetSingleValueByName(\"test1\");\n"
        "\n"
        "    ReadValue = value;\n"
        "}\n"
        "\n"
        "bool RunTest(Delegate@ delegate){\n"
        "\n"
        "    delegate.Register(@Callback);\n"
        "\n"
        "    NamedVars@ variables = NamedVars();\n"
        "    variables.AddValue(ScriptSafeVariableBlock(\"test1\", 12));\n"
        "    \n"
        "    delegate.Call(variables);\n"
        "\n"
        "    return ReadValue == 12;\n"
        "}"
    );

    mod->AddScriptSegment(sourcecode);

    auto module = mod->GetModule();

    REQUIRE(module != nullptr);

    Delegate* basicDelegate = new Delegate();

    ScriptRunningSetup ssetup;
    ssetup.SetEntrypoint("RunTest").SetUseFullDeclaration(false);

    auto returned = exec.RunScript<bool>(mod, ssetup, basicDelegate);

    CHECK(ssetup.ScriptExisted == true);
    REQUIRE(returned.Result == SCRIPT_RUN_RESULT::Success);

    // check did it exist //
    bool value = returned.Value;

    CHECK(value);
    CHECK(basicDelegate->GetRefCount() == 1);
    basicDelegate->Release();
}


