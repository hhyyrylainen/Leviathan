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
    Logger log("Test/ExtraTest.txt");

    SECTION("minimal syntax string") {

        // Try to parse a minimal syntax file //
        auto ofile = ObjectFileProcessor::ProcessObjectFileFromString(BasicTestStr,
            "basic in memory test", &reporter);

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
        CHECK(ObjectFileProcessor::LoadValueFromNamedVars(list->GetVariables(),
                "firstValue", firstValue, 0));

        CHECK(firstValue == 1);
    }

    SECTION("Just an empty object") {
        constexpr auto ObjToParse = "\n"
            "o Type \"obj1\"{\n"
            "\n"
            "}\n";

        auto ofile = ObjectFileProcessor::ProcessObjectFileFromString(ObjToParse,
            "ObjToParse", &reporter);

        REQUIRE(ofile != nullptr);
        REQUIRE(ofile->GetTotalObjectCount() == 1);
    }
}

#ifndef LEVIATHAN_UE_PLUGIN
TEST_CASE("ObjectFiles parser read test file", "[objectfile]"){

    DummyReporter reporter;
    ScriptExecutor exec;
    PartialEngine<false> engine;
    
    // First test the minimal file //
    string minfile = "Data/Scripts/tests/SimpleTest.levof";

    // Try to parse it //
    auto ofile = ObjectFileProcessor::ProcessObjectFile(minfile, &reporter);

    REQUIRE(ofile != nullptr);

    const NamedVars& HeaderVars = *ofile->GetVariables();

    // Validate the output //
    CHECK(HeaderVars.GetVariableCount() == 4);

    string TestFile = "Data/Scripts/tests/TestObjectFile.levof";
    
    // Make sure the loading is correct //
    auto rofile = ObjectFileProcessor::ProcessObjectFile(TestFile, &reporter);

    REQUIRE(rofile != nullptr);

    // TODO: add rest of tests
}
#endif //LEVIATHAN_UE_PLUGIN

TEST_CASE("Allow missing ending ';' in objectfile", "[objectfile]") {

    DummyReporter reporter;

    SECTION("Header var") {

        auto ofile = ObjectFileProcessor::ProcessObjectFileFromString("Basic = 12",
            "missing ; parse test", &reporter);

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

    SECTION("In variable list") {

        constexpr auto File = "o a \"obj\"{\n"
            "    \n"
            "    l values{\n"
            "        is_default: true\n"
            "        chat_prefix: \"[NEW] \"\n"
            "        auto_promote: \"1hr\";\n"
            "    }\n"
            "}";

        auto ofile = ObjectFileProcessor::ProcessObjectFileFromString(File, "parse test",
            &reporter);

        REQUIRE(ofile != nullptr);

        REQUIRE(ofile->GetTotalObjectCount() == 1);
        REQUIRE(ofile->GetObject(0)->GetListWithName("values"));

        const auto& variables = ofile->GetObject(0)->GetListWithName("values")->GetVariables();

        REQUIRE(variables.GetValueDirect("is_default"));
        CHECK(variables.GetValueDirect("is_default")->GetCommonType() == DATABLOCK_TYPE_BOOL);
        CHECK(variables.GetValueDirect("is_default")->GetValue().
            ConvertAndReturnVariable<bool>() == true);

        REQUIRE(variables.GetValueDirect("chat_prefix"));
        CHECK(variables.GetValueDirect("chat_prefix")->GetCommonType() ==
            DATABLOCK_TYPE_STRING);
        CHECK(variables.GetValueDirect("chat_prefix")->GetValue().
            ConvertAndReturnVariable<std::string>() == "[NEW] ");

        REQUIRE(variables.GetValueDirect("auto_promote"));
        CHECK(variables.GetValueDirect("auto_promote")->GetCommonType() ==
            DATABLOCK_TYPE_STRING);
        CHECK(variables.GetValueDirect("auto_promote")->GetValue().
            ConvertAndReturnVariable<std::string>() == "1hr");
    }
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

        auto ofile = ObjectFileProcessor::ProcessObjectFileFromString(serialized,
            "serialized", &reporter);

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

        list1->AddVariable(std::make_shared<NamedVariableList>("boring variable",
                new VariableBlock(true)));
        list1->AddVariable(std::make_shared<NamedVariableList>("another var",
                new VariableBlock(4)));

        obj1->AddVariableList(std::unique_ptr<ObjectFileList>(list1.release()));

        auto text1 = std::make_unique<ObjectFileTextBlockProper>("Just a list");
        text1->AddTextLine("This is the place for a cool");
        text1->AddTextLine("story that spans multiple lines");

        obj1->AddTextBlock(std::unique_ptr<ObjectFileTextBlock>(text1.release()));

        CHECK(prefixes.size() == 0);

        prefixes.push_back(std::make_unique<std::string>("3"));
        prefixes.push_back(std::make_unique<std::string>("id(25)"));
        auto obj2 = std::make_shared<ObjectFileObjectProper>("A fine object", "Gui",
            std::move(prefixes));

        obj.AddObject(obj1);
        obj.AddObject(obj2);

        obj.AddNamedVariable(std::make_shared<NamedVariableList>("TestVar",
                new VariableBlock(17.5)));
        obj.AddNamedVariable(std::make_shared<NamedVariableList>("Var2",
                new VariableBlock(std::string("things"))));

        std::string serialized;
        REQUIRE(ObjectFileProcessor::SerializeObjectFile(obj, serialized));

        CHECK(serialized.size() > 0);

        auto ofile = ObjectFileProcessor::ProcessObjectFileFromString(serialized, "serialized",
            &reporter);

        REQUIRE(ofile);
        REQUIRE(ofile->GetTotalObjectCount() == 2);
        CHECK(ofile->GetObject(0)->GetName() == "obj1");
        CHECK(ofile->GetObject(0)->GetTypeName() == "Type");
        REQUIRE(ofile->GetObject(0)->GetPrefixesCount() == 1);
        CHECK(ofile->GetObject(0)->GetPrefix(0) == "prefix1");
        REQUIRE(ofile->GetObject(0)->GetListWithName("Just a list"));
        CHECK(ofile->GetObject(0)->GetListWithName("Just a list")->GetVariables().
            GetValueDirect("another var"));

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

        auto ofile = ObjectFileProcessor::ProcessObjectFileFromString(serialized,
            "serialized", &reporter);

        REQUIRE(ofile);
        REQUIRE(ofile->GetTotalObjectCount() == 1);
        CHECK(ofile->GetObject(0)->GetName() == "ObjName");
        CHECK(ofile->GetObject(0)->GetTypeName() == "Type");
        REQUIRE(ofile->GetObject(0)->GetPrefixesCount() == 3);
        REQUIRE(ofile->GetObject(0)->GetPrefix(1) == "second");
        REQUIRE(ofile->GetObject(0)->GetPrefix(2) == "prefix");
    }
}


TEST_CASE("Fabricators permissions parse test", "[objectfile]") {

    constexpr auto File = "permissions_version: 1;\n"
        "\n"
        "o Rank \"default\"{\n"
        "    \n"
        "    l values{\n"
        "        is_default: true\n"
        "        chat_prefix: \"[NEW] \"\n"
        "        auto_promote: \"1hr\"\n"
        "    }\n"
        "    \n"
        "    l nodes{\n"
        "        core.tp.request: true\n"
        "        core.home: true\n"
        "        core.kill.self: true\n"
        "        core.kill: default\n"
        "        core.kill.other: false\n"
        "    }\n"
        "}";

    DummyReporter reporter;
    auto ofile = ObjectFileProcessor::ProcessObjectFileFromString(File,
        "permissions_test1", &reporter);

    REQUIRE(ofile != nullptr);

    REQUIRE(ofile->GetTotalObjectCount() == 1);
    REQUIRE(ofile->GetObject(0)->GetListWithName("values"));
    REQUIRE(ofile->GetObject(0)->GetListWithName("nodes"));

    const auto& variables = ofile->GetObject(0)->GetListWithName("values")->GetVariables();

    REQUIRE(variables.GetValueDirect("is_default"));
    CHECK(variables.GetValueDirect("is_default")->GetValue().ConvertAndReturnVariable<bool>()
        == true);

    REQUIRE(variables.GetValueDirect("chat_prefix"));
    CHECK(variables.GetValueDirect("chat_prefix")->GetValue().
        ConvertAndReturnVariable<std::string>() == "[NEW] ");


    const auto& nodes = ofile->GetObject(0)->GetListWithName("nodes")->GetVariables();

    REQUIRE(nodes.GetValueDirect("core.home"));
    CHECK(nodes.GetValueDirect("core.home")->GetValue().ConvertAndReturnVariable<bool>()
        == true);

    REQUIRE(nodes.GetValueDirect("core.kill"));
    CHECK(nodes.GetValueDirect("core.kill")->GetValue().ConvertAndReturnVariable<std::string>()
        == "default");

}
