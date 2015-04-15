#include "ObjectFiles/ObjectFileProcessor.h"
#include "Script/ScriptExecutor.h"

#include "catch.hpp"

using namespace Leviathan;


TEST_CASE("ObjectFiles parser read test file", "[objectfile]"){

    ScriptExecutor exec;
    
	// First test the minimal file //
	wstring minfile = L"Data/Scripts/tests/SimpleTest.levof";

	// Try to parse it //
	auto ofile = ObjectFileProcessor::ProcessObjectFile(minfile);

    REQUIRE(ofile != nullptr);

	const NamedVars& HeaderVars = *ofile->GetVariables();

	// Validate the output //
    CHECK(HeaderVars.GetVariableCount() == 4);

    wstring TestFile = L"Data/Scripts/tests/TestObjectFile.levof";
	
	// Make sure the loading is correct //
	auto rofile = ObjectFileProcessor::ProcessObjectFile(TestFile);

    REQUIRE(rofile != nullptr);

    // TODO: add rest of tests
}
