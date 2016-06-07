#include "ObjectFiles/ObjectFileProcessor.h"

#ifndef LEVIATHAN_UE_PLUGIN
#include "Script/ScriptExecutor.h"
#include "PartialEngine.h"
#endif //LEVIATHAN_UE_PLUGIN

#include "catch.hpp"
#include "../DummyLog.h"

using namespace Leviathan;
using namespace std;

constexpr auto BasicTestStr = "FirstVariable = 42;\n"
    "Use-Something = true;\n"
    "A string? = \"hello\";\n"
    "more: false;\n"
    "\n"
    "o TestType \"First object\"{\n"
    "    l list {\n"
    "        firstValue = 1;\n"
    "        secondValue = \"2\";\n"
    "    }\n"
    "}";


TEST_CASE("ObjectFiles parser basic in-memory test", "[objectfile]") {

    DummyReporter reporter;

    // Try to parse a minimal syntax file //
    auto ofile = ObjectFileProcessor::ProcessObjectFileFromString(BasicTestStr, "basic in memory test", &reporter);

    REQUIRE(ofile != nullptr);

    const NamedVars& HeaderVars = *ofile->GetVariables();

    // Validate the output //
    CHECK(HeaderVars.GetVariableCount() == 4);

    REQUIRE(ofile->GetTotalObjectCount() == 1);

    ObjectFileObject* obj = ofile->GetObjectFromIndex(0);

    REQUIRE(obj != nullptr);

    CHECK(obj->GetName() == "First object");

    ObjectFileList* list = obj->GetList(0);

    REQUIRE(list != nullptr);

    auto value = list->GetVariables().GetValueDirect("firstValue");

    REQUIRE(value);

    int firstValue;
    CHECK(ObjectFileProcessor::LoadValueFromNamedVars(list->GetVariables(), "firstValue", firstValue, 0));

    CHECK(firstValue == 1);
}

#ifndef LEVIATHAN_UE_PLUGIN
TEST_CASE("ObjectFiles parser read test file", "[objectfile]"){

    ScriptExecutor exec;
    PartialEngine<false, NETWORKED_TYPE_CLIENT> engine;
    
	// First test the minimal file //
	string minfile = "Data/Scripts/tests/SimpleTest.levof";

	// Try to parse it //
	auto ofile = ObjectFileProcessor::ProcessObjectFile(minfile);

    REQUIRE(ofile != nullptr);

	const NamedVars& HeaderVars = *ofile->GetVariables();

	// Validate the output //
    CHECK(HeaderVars.GetVariableCount() == 4);

    string TestFile = "Data/Scripts/tests/TestObjectFile.levof";
	
	// Make sure the loading is correct //
	auto rofile = ObjectFileProcessor::ProcessObjectFile(TestFile);

    REQUIRE(rofile != nullptr);

    // TODO: add rest of tests
}
#endif //LEVIATHAN_UE_PLUGIN