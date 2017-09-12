#include "../PartialEngine.h"

#include "Script/ScriptExecutor.h"
#include "Script/ScriptModule.h"
#include "Handlers/IDFactory.h"

#include "catch.hpp"

using namespace Leviathan;
using namespace Leviathan::Test;

TEST_CASE("Basic script running", "[script]"){

    PartialEngine<false> engine;
    
    IDFactory ids;
    ScriptExecutor exec;

	// setup the script //
	auto mod = exec.CreateNewModule("TestScript", "ScriptGenerator").lock();

    // Setup source for script //
    auto sourcecode = std::make_shared<ScriptSourceFileData>("Script.cpp", __LINE__ + 1,
        "int TestFunction(int Val1, int Val2){\n"
		"// do some time consuming stuff //\n"
		"Val1 *= Val1+Val2 % 15;\n"
		"Val2 /= Val2-Val1-Val2*25+2;\n"
		"return 42;\n"
		"}");

    mod->AddScriptSegment(sourcecode);

    auto module = mod->GetModule();

    REQUIRE(module != nullptr);

    std::vector<std::shared_ptr<NamedVariableBlock>> Params = {
        std::make_shared<NamedVariableBlock>(252134, "Val1"),
        std::make_shared<NamedVariableBlock>(25552, "Val2")
    };

    ScriptRunningSetup ssetup;
    ssetup.SetArguments(Params).SetEntrypoint("TestFunction").SetUseFullDeclaration(false)
        //.SetPrintErrors(false)
        ;

    std::shared_ptr<VariableBlock> returned = exec.RunSetUp(mod.get(), &ssetup);

    CHECK(ssetup.ScriptExisted == true);

    // check did it exist //
    int Value = *returned;

    CHECK(Value == 42);

	mod->DeleteThisModule();
}

TEST_CASE("Creating events in scripts", "[script][event]"){

    PartialEngine<false> engine;
    
    IDFactory ids;
    ScriptExecutor exec;

	// setup the script //
	auto mod = exec.CreateNewModule("TestScript", "ScriptGenerator").lock();

    // Setup source for script //
    auto sourcecode = std::make_shared<ScriptSourceFileData>("Script.cpp", __LINE__ + 1,
        "int TestFunction(){\n"
        "    GenericEvent@ event = GenericEvent(\"TestEvent\");\n"
        "\n"
        "    NamedVars@ tempvalues = event.GetNamedVars();\n"
        "    if(tempvalues is null)\n"
        "         return 0;\n"
        "    // Next four lines aren't good style //\n"
        "    NamedVars@ otherVals = @event.GetNamedVars();\n"
        "    if(@tempvalues !is @otherVals)\n"
        "         return 0;\n"
        "    if(@tempvalues is null)\n"
        "         return 0;\n"        
        "    if(tempvalues !is otherVals)\n"
        "         return 0;\n"
        "\n"
        "    return 12;\n"
        "}"
    );

    mod->AddScriptSegment(sourcecode);

    auto module = mod->GetModule();

    REQUIRE(module != nullptr);

    ScriptRunningSetup ssetup;
    ssetup.SetEntrypoint("TestFunction").SetUseFullDeclaration(false);

    std::shared_ptr<VariableBlock> returned = exec.RunSetUp(mod.get(), &ssetup);

    CHECK(ssetup.ScriptExisted == true);

    // check did it exist //
    int Value = *returned;

    CHECK(Value == 12);
}


TEST_CASE("Documentation samples compile", "[script]"){

    PartialEngine<false> engine;
    
    IDFactory ids;
    ScriptExecutor exec;

	// setup the script //
	auto mod = exec.CreateNewModule("TestScript", "ScriptGenerator").lock();

    // Setup source for script //
    auto sourcecode = std::make_shared<ScriptSourceFileData>("Script.cpp", __LINE__ + 1,
        "NamedVars@ values = NamedVars();\n"
        "\n"
        "NamedVars@ GetVars(){\n"
        "\n"
        "    return values;\n"
        "}\n"
        "\n"
        "void Sample1(){\n"
        "\n"
        "    if(values is null)\n"
        "        return;\n"
        "\n"
        "    NamedVars@ other;\n"
        "\n"
        "    if(values is other)\n"
        "        return;\n"
        "\n"
        "    @other = @values;\n"
        "\n"
        "    NamedVars@ thirdVariable = GetVars();\n"
        "\n"
        "    @other = null;\n"
        "}\n"
        "\n"
        "NamedVars@ values2 = @NamedVars();\n"
        "\n"
        "void Sample2(){\n"
        "\n"
        "    if(@values2 is null)\n"
        "        return;\n"
        "\n"
        "    NamedVars@ other;\n"
        "\n"
        "    if(@values2 is @other)\n"
        "        return;\n"
        "\n"
        "    @other = @values;\n"
        "\n"
        "    NamedVars@ thirdVariable = GetVars();\n"
        "\n"
        "    @other = null;\n"
        "}"
    );

    mod->AddScriptSegment(sourcecode);

    auto module = mod->GetModule();

    REQUIRE(module != nullptr);

    ScriptRunningSetup ssetup;
    ssetup.SetEntrypoint("Sample1").SetUseFullDeclaration(false);

    exec.RunSetUp(mod.get(), &ssetup);

    CHECK(ssetup.ScriptExisted == true);

    ssetup.SetEntrypoint("Sample2");

    exec.RunSetUp(mod.get(), &ssetup);

    CHECK(ssetup.ScriptExisted == true);    
}
