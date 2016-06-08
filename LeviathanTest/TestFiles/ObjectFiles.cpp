#include "ObjectFiles/ObjectFileProcessor.h"

#ifndef LEVIATHAN_UE_PLUGIN
#include "Script/ScriptExecutor.h"
#include "PartialEngine.h"
#endif //LEVIATHAN_UE_PLUGIN

#include "catch.hpp"
#include "../DummyLog.h"
#include "Logger.h"

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
    Logger log("Test/TestLog.txt");

    SECTION("minimal syntax string") {

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

    SECTION("Just an empty object") {
        constexpr auto ObjToParse = "\n"
            "o Type \"obj1\"{\n"
            "\n"
            "}\n";

        auto ofile = ObjectFileProcessor::ProcessObjectFileFromString(ObjToParse, "ObjToParse", &reporter);

        REQUIRE(ofile != nullptr);
        REQUIRE(ofile->GetTotalObjectCount() == 1);
    }
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

TEST_CASE("Allow missing ending ';' in objectfile", "[objectfile]") {

    DummyReporter reporter;
    auto ofile = ObjectFileProcessor::ProcessObjectFileFromString("Basic = 12", "missing ; parse test", &reporter);

    REQUIRE(ofile != nullptr);

    const NamedVars& HeaderVars = *ofile->GetVariables();

    // Validate the output //
    CHECK(HeaderVars.GetVariableCount() == 1);

    CHECK(HeaderVars);

    REQUIRE(HeaderVars.Find("Basic") < HeaderVars.GetVariableCount());

    int number;
    CHECK(HeaderVars.GetValueAndConvertTo("Basic", number));

    CHECK(number == 12);
}

TEST_CASE("Object file saving", "[objectfile]") {

    DummyReporter reporter;

    SECTION("Basic file with one object") {

        ObjectFile obj;
        obj.AddObject(std::make_shared<ObjectFileObjectProper>("obj1", "Type", 
            std::vector<std::unique_ptr<std::string>>()));

        std::string serialized;
        REQUIRE(ObjectFileProcessor::SerializeObjectFile(obj, serialized));

        CHECK(serialized.size() > 0);

        auto ofile = ObjectFileProcessor::ProcessObjectFileFromString(serialized, "serialized", &reporter);

        REQUIRE(ofile);
        REQUIRE(ofile->GetTotalObjectCount() == 1);
        CHECK(ofile->GetObject(0)->GetName() == "obj1");
        CHECK(ofile->GetObject(0)->GetTypeName() == "Type");
    }

    SECTION("Basic full example") {

        ObjectFile obj;

        std::vector<std::unique_ptr<std::string>> prefixes;
        prefixes.push_back(std::make_unique<std::string>("prefix1"));
        auto obj1 = std::make_shared<ObjectFileObjectProper>("obj1", "Type",
            std::move(prefixes));

        auto list1 = std::make_unique<ObjectFileListProper>("Just a list");

        list1->AddVariable(std::make_shared<NamedVariableList>("boring variable", new VariableBlock(true)));
        list1->AddVariable(std::make_shared<NamedVariableList>("another var", new VariableBlock(4)));

        obj1->AddVariableList(std::move(std::unique_ptr<ObjectFileList>(list1.release())));

        auto text1 = std::make_unique<ObjectFileTextBlockProper>("Just a list");
        text1->AddTextLine("This is the place for a cool");
        text1->AddTextLine("story that spans multiple lines");

        obj1->AddTextBlock(std::move(std::unique_ptr<ObjectFileTextBlock>(text1.release())));

        CHECK(prefixes.size() == 0);

        prefixes.push_back(std::make_unique<std::string>("3"));
        prefixes.push_back(std::make_unique<std::string>("id(25)"));
        auto obj2 = std::make_shared<ObjectFileObjectProper>("A fine object", "Gui",
            std::move(prefixes));

        obj.AddObject(obj1);
        obj.AddObject(obj2);

        obj.AddNamedVariable(std::make_shared<NamedVariableList>("TestVar", new VariableBlock(17.5)));
        obj.AddNamedVariable(std::make_shared<NamedVariableList>("Var2", new VariableBlock(std::string("things"))));

        std::string serialized;
        REQUIRE(ObjectFileProcessor::SerializeObjectFile(obj, serialized));

        CHECK(serialized.size() > 0);

        auto ofile = ObjectFileProcessor::ProcessObjectFileFromString(serialized, "serialized", &reporter);

        REQUIRE(ofile);
        REQUIRE(ofile->GetTotalObjectCount() == 2);
        CHECK(ofile->GetObject(0)->GetName() == "obj1");
        CHECK(ofile->GetObject(0)->GetTypeName() == "Type");
        REQUIRE(ofile->GetObject(0)->GetPrefixesCount() == 1);
        CHECK(ofile->GetObject(0)->GetPrefix(0) == "prefix1");
        REQUIRE(ofile->GetObject(0)->GetListWithName("Just a list"));
        CHECK(ofile->GetObject(0)->GetListWithName("Just a list")->GetVariables().GetValueDirect("another var"));

        CHECK(ofile->GetObject(1)->GetName() == "A fine object");
        CHECK(ofile->GetObject(1)->GetTypeName() == "Gui");
        REQUIRE(ofile->GetObject(1)->GetPrefixesCount() == 2);
        CHECK(ofile->GetObject(1)->GetPrefix(0) == "3");
        CHECK(ofile->GetObject(1)->GetPrefix(1) == "id(25)");

        NamedVars* vars = ofile->GetVariables();

        REQUIRE(vars);
        CHECK(vars->GetVariableCount() == 2);

        string testvalue;

        REQUIRE(vars->GetValueAndConvertTo("Var2", testvalue));
        CHECK(testvalue == "things");

    }

    SECTION("Space in prefix not quoted") {

        ObjectFile obj;

        std::vector<std::unique_ptr<std::string>> prefixes;
        prefixes.push_back(std::make_unique<std::string>("first"));
        prefixes.push_back(std::make_unique<std::string>("second prefix"));

        obj.AddObject(std::make_shared<ObjectFileObjectProper>("ObjName", "Type",
            std::move(prefixes)));

        std::string serialized;
        REQUIRE(ObjectFileProcessor::SerializeObjectFile(obj, serialized));

        CHECK(serialized.size() > 0);

        auto ofile = ObjectFileProcessor::ProcessObjectFileFromString(serialized, "serialized", &reporter);

        REQUIRE(ofile);
        REQUIRE(ofile->GetTotalObjectCount() == 1);
        CHECK(ofile->GetObject(0)->GetName() == "ObjName");
        CHECK(ofile->GetObject(0)->GetTypeName() == "Type");
        REQUIRE(ofile->GetObject(0)->GetPrefixesCount() == 3);
        REQUIRE(ofile->GetObject(0)->GetPrefix(1) == "second");
        REQUIRE(ofile->GetObject(0)->GetPrefix(2) == "prefix");
    }
}