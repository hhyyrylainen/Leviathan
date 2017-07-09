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
    auto sourcecode = std::make_shared<ScriptSourceFileData>("memory_test_script", 1,
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
