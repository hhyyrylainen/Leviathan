#include "Script/ScriptExecutor.h"
#include "Script/ScriptModule.h"

#include "catch.hpp"

#include <boost/assign.hpp>

using namespace Leviathan;
using namespace std;

TEST_CASE("Basic script running", "[script]"){


    auto exec(move(unique_ptr<ScriptExecutor>(new ScriptExecutor())));

	// setup the script //
	auto mod = exec->CreateNewModule("TestScrpt", "ScriptGenerator").lock();

    // Setup source for script //
    auto sourcecode = make_shared<ScriptSourceFileData>("memory_test_script", 1,
        "int TestFunction(int Val1, int Val2){\n"
		"// do some time consuming stuff //\n"
		"Val1 *= Val1+Val2 % 15;\n"
		"Val2 /= Val2-Val1-Val2*25+2;\n"
		"return 42;\n"
		"}");

    mod->AddScriptSegment(sourcecode);

    auto module = mod->GetModule();

    REQUIRE(module != nullptr);

	vector<shared_ptr<NamedVariableBlock>> Params = boost::assign::list_of(new NamedVariableBlock(
            new IntBlock(252134), "Val1"))
		(new NamedVariableBlock(new IntBlock(25552), "Val2"));

    ScriptRunningSetup ssetup;
    ssetup.SetArguments(Params).SetEntrypoint("TestFunction").SetUseFullDeclaration(false)
        //.SetPrintErrors(false)
        ;

    shared_ptr<VariableBlock> returned = exec->RunSetUp(mod.get(), &ssetup);

    CHECK(ssetup.ScriptExisted == true);

    // check did it exist //
    int Value = *returned;

    CHECK(Value == 42);
        

	mod->DeleteThisModule();
}
