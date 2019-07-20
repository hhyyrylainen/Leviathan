#include "Common/DataStoring/NamedVars.h"

#include "GUI/CEFConversionHelpers.h"
#include "ObjectFiles/ObjectFileProcessor.h"

#include "../DummyLog.h"
#include "catch.hpp"

using namespace Leviathan;
using namespace Leviathan::Test;

TEST_CASE("NamedVars creation and value retrieve", "[variable]")
{
    std::vector<std::shared_ptr<NamedVariableList>> Variables;
    Variables.push_back(std::shared_ptr<NamedVariableList>(
        new NamedVariableList("varone", new VariableBlock(1))));

    Variables.push_back(std::shared_ptr<NamedVariableList>(
        new NamedVariableList("var two", new VariableBlock(2))));

    Variables.push_back(std::shared_ptr<NamedVariableList>(
        new NamedVariableList("var", new VariableBlock(3))));

    Variables.push_back(std::shared_ptr<NamedVariableList>(
        new NamedVariableList("var4", new VariableBlock(4))));

    Variables.push_back(std::shared_ptr<NamedVariableList>(
        new NamedVariableList("var5", new VariableBlock(5))));

    // create holder //
    NamedVars holder = NamedVars();

    holder.SetVec(Variables);

    REQUIRE(holder.GetValue("var5"));

    // add some more values //
    SECTION("Adding and removing")
    {
        holder.AddVar("var66", new VariableBlock(25));
        holder.Remove(holder.Find("var two"));

        CHECK(holder.Find("var66") < holder.GetVariableCount());
        CHECK(holder.Find("var two") >= holder.GetVariableCount());
    }

    SECTION("Variable types")
    {
        CHECK(holder.GetValue("var5")->GetBlockConst()->Type == DATABLOCK_TYPE_INT);
    }

    SECTION("Value retrieve")
    {
        int checkval;

        REQUIRE(holder.GetValueAndConvertTo<int>("var", checkval) == true);

        CHECK(checkval == 3);
    }
}

TEST_CASE("NamedVars line parsing", "[variable][objectfiles]")
{
    DummyReporter reporter;

    SECTION("Line 'this= not this'")
    {
        std::string typelinething = "this= not this";

        // creation testing //
        auto ptry = std::make_shared<NamedVariableList>(typelinething, &reporter);

        std::wstring emptystr;

        REQUIRE(ptry->GetValueDirect()->ConvertAndAssingToVariable<std::wstring>(emptystr) ==
                true);

        CHECK(emptystr == L"not this");
    }

    SECTION("Line 'this=2'")
    {
        std::string typelinething = "this=2";

        auto result = std::shared_ptr<NamedVariableList>(
            new NamedVariableList(typelinething, &reporter));

        int checkval;

        REQUIRE(result->GetValueDirect()->ConvertAndAssingToVariable<int>(checkval) == true);

        CHECK(checkval == 2);
    }

    SECTION("Line 'oh=2;'")
    {
        std::string line = "oh=2;";

        auto result = std::make_shared<NamedVariableList>(line, &reporter);

        REQUIRE(result);

        CHECK(result->GetName() == "oh");
    }

    SECTION("Basic bracket expression")
    {
        NamedVariableList advlist("value = [first, 2]", &reporter);

        REQUIRE(advlist.GetVariableCount() == 2);

        CHECK(advlist.GetValueDirect(0)->GetBlockConst()->Type == DATABLOCK_TYPE_STRING);

        CHECK(advlist.GetValueDirect(1)->GetBlockConst()->Type == DATABLOCK_TYPE_INT);
    }

    SECTION("Advanced line 'Color = [[0.1], [4], [true], [\"lol\"]]'")
    {
        std::string linething = "Color = [[0.1], [4], [true], [\"lol\"]]";
        NamedVariableList advlist(linething, &reporter);

        REQUIRE(advlist.GetVariableCount() == 4);

        CHECK(advlist.GetValueDirect(0)->GetBlockConst()->Type == DATABLOCK_TYPE_FLOAT);

        CHECK(advlist.GetValueDirect(1)->GetBlockConst()->Type == DATABLOCK_TYPE_INT);

        CHECK(advlist.GetValueDirect(2)->GetBlockConst()->Type == DATABLOCK_TYPE_BOOL);

        CHECK(advlist.GetValueDirect(3)->GetBlockConst()->Type == DATABLOCK_TYPE_STRING);
    }
}

#ifdef SFML_PACKETS
TEST_CASE("NamedVars packet serialization", "[variable]")
{
    NamedVars packettestorig;

    packettestorig.AddVar("MyVar1", new VariableBlock((std::string) "std::string_block"));
    packettestorig.AddVar("Secy", new VariableBlock(true));

    // Add to packet //
    sf::Packet packetdata;

    packettestorig.AddDataToPacket(packetdata);

    // Read from a packet //
    NamedVars frompacket(packetdata);

    REQUIRE(frompacket.GetVec()->size() == packettestorig.GetVec()->size());

    // Check values //
    auto datablock = frompacket.GetValue("MyVar1");

    REQUIRE(datablock);

    CHECK(datablock->GetBlockConst()->Type == DATABLOCK_TYPE_STRING);

    VariableBlock receiver2;
    frompacket.GetValue(1, receiver2);

    CHECK(static_cast<bool>(receiver2) == true);
}
#endif // SFML_PACKETS

TEST_CASE("Specific value parsing", "[variable]")
{
    DummyReporter reporter;

    SECTION("Width = 1280;")
    {
        NamedVariableList var("Width = 1280;", &reporter);

        REQUIRE(var.GetVariableCount() == 1);
        CHECK(var.GetCommonType() == DATABLOCK_TYPE_INT);
        CHECK(var.GetValue(0).operator int() == 1280);

        SECTION("Std::Stringification")
        {

            CHECK(var.ToText(0, true) == "Width = [[1280]];");
            CHECK(var.ToText(1, true) == "Width: [[1280]];");


            SECTION("Parsing back from std::string")
            {

                NamedVariableList var2(var.ToText(0), &reporter);

                REQUIRE(var2.GetVariableCount() == 1);
                CHECK(var2.GetCommonType() == DATABLOCK_TYPE_INT);
                CHECK(var2.GetValue(0).operator int() == 1280);
            }
        }
    }

    SECTION("Engine conf sample and variable loading")
    {
        NamedVars values("Width = 1280;\n"
                         "Height = [[720]];\n"
                         "Windowed = [true];\n"
                         "RenderSystemName = [[[\"Open.*GL\"]]];\n",
            &reporter);

        int width;
        int height;
        bool window;
        std::string rendersystemname;

        ObjectFileProcessor::LoadValueFromNamedVars(values, "Width", width, 0);

        CHECK(width == 1280);

        ObjectFileProcessor::LoadValueFromNamedVars(values, "Height", height, 0);

        CHECK(height == 720);

        ObjectFileProcessor::LoadValueFromNamedVars(values, "Windowed", window, false);

        CHECK(window == true);

        ObjectFileProcessor::LoadValueFromNamedVars(
            values, "RenderSystemName", rendersystemname, std::string(""));

        CHECK(rendersystemname == "Open.*GL");
    }

    SECTION("DataStore things")
    {
        NamedVariableList var("StartCount = [[1]];", &reporter);

        REQUIRE(var.GetVariableCount() == 1);
        CHECK(var.GetCommonType() == DATABLOCK_TYPE_INT);
        CHECK(var.GetValue().operator int() == 1);

        var = NamedVariableList(
            "StartCount", new VariableBlock(new IntBlock(var.GetValue().operator int() + 1)));

        CHECK(var.GetCommonType() == DATABLOCK_TYPE_INT);
        CHECK(var.GetValue().operator int() == 2);
        CHECK(var.ToText(0, true) == "StartCount = [[2]];");
        CHECK(var.ToText(0, false) == "StartCount = 2;");
        CHECK(var.ToText(1, false) == "StartCount: 2;");


        var = NamedVariableList(
            "StartCount", new VariableBlock(new IntBlock(var.GetValue().operator int() + 1)));

        CHECK(var.GetCommonType() == DATABLOCK_TYPE_INT);
        CHECK(var.GetValue().operator int() == 3);
        CHECK(var.ToText(0, true) == "StartCount = [[3]];");
        CHECK(var.ToText(0, false) == "StartCount = 3;");
    }
}

TEST_CASE("Converting empty std::string blocks", "[variable][datablock]")
{
    NamedVariableList empty("name", new StringBlock(new std::string()));

    CHECK(empty.GetValue(0).operator int() == 0);
    CHECK(empty.GetValue(0).operator char() == 0);
    CHECK(empty.GetValue(0).operator float() == 0.f);
    CHECK(empty.GetValue(0).operator double() == 0.0);
    CHECK(empty.GetValue(0).operator std::string() == "");
    CHECK(empty.GetValue(0).operator std::wstring() == L"");
    CHECK(empty.GetValue(0).operator bool() == false);
}

TEST_CASE("Allow missing ending';'", "[variable][datablock]")
{
    DummyReporter dummy;
    NamedVariableList testlist("name = 13.5", &dummy);

    REQUIRE(testlist.IsValid());
    REQUIRE(testlist.GetVariableCount() == 1);

    REQUIRE(testlist.GetValue().IsConversionAllowedNonPtr<float>());

    CHECK(static_cast<float>(testlist.GetValue()) == 13.5f);
}

TEST_CASE("Verify equals operator works", "[variable][datablock]")
{
    SECTION("Directly with VariableBlocks")
    {
        CHECK(VariableBlock(static_cast<DataBlockAll*>(nullptr)) !=
              VariableBlock(static_cast<DataBlockAll*>(nullptr)));

        CHECK(VariableBlock(new IntBlock(2)) !=
              VariableBlock(static_cast<DataBlockAll*>(nullptr)));
        CHECK(VariableBlock(new IntBlock(2)) != VariableBlock(new IntBlock(3)));
        CHECK(VariableBlock(new IntBlock(2)) != VariableBlock(new StringBlock("3")));

        CHECK(VariableBlock(new IntBlock(2)) == VariableBlock(new IntBlock(2)));
        CHECK(!(VariableBlock(new IntBlock(2)) != VariableBlock(new IntBlock(2))));

        CHECK(VariableBlock(std::string("how")) == VariableBlock(std::string("how")));
        CHECK(VariableBlock(std::string("how")) != VariableBlock(std::string("how2")));
    }

    SECTION("Through NamedVariableList")
    {
        CHECK(NamedVariableList("val") == NamedVariableList("val"));
        CHECK(!(NamedVariableList("val") != NamedVariableList("val")));

        CHECK(
            NamedVariableList("val", new VariableBlock(static_cast<DataBlockAll*>(nullptr))) !=
            NamedVariableList("val", new VariableBlock(static_cast<DataBlockAll*>(nullptr))));

        CHECK(NamedVariableList("val", new VariableBlock(2)) !=
              NamedVariableList("val", new VariableBlock(1)));

        CHECK(NamedVariableList("val", new VariableBlock(1)) ==
              NamedVariableList("val", new VariableBlock(1)));

        CHECK(!(NamedVariableList("val", new VariableBlock(1)) !=
                NamedVariableList("val", new VariableBlock(1))));

        // Different names //
        CHECK(NamedVariableList("val2", new VariableBlock(1)) !=
              NamedVariableList("val", new VariableBlock(1)));
    }
}

TEST_CASE("Parsing values back from ToText", "[variable][datablock]")
{
    DummyReporter dummy;

    SECTION("Empty value into empty std::string")
    {
        NamedVariableList original("val");

        const auto asstring = original.ToText(0, false);

        CHECK(asstring.length() > 0);

        NamedVariableList returned(asstring, &dummy);

        CHECK(returned.GetName() == original.GetName());
        CHECK(returned.GetValueDirect() != original.GetValueDirect());
        CHECK(returned.GetValueDirect());
    }

    SECTION("empty std::string")
    {
        NamedVariableList original("val", new VariableBlock(std::string("")));

        const auto asstring = original.ToText(0, true);

        CHECK(asstring.length() > 0);

        NamedVariableList returned(asstring, &dummy);

        CHECK(returned.GetName() == original.GetName());
        REQUIRE(returned.GetValueDirect());
        REQUIRE(original.GetValueDirect());
        CHECK(returned.GetValue() == original.GetValue());
    }

    SECTION("a simple std::string")
    {
        NamedVariableList original("val", new VariableBlock(std::string("arc")));

        const auto asstring = original.ToText(0, true);

        CHECK(asstring.length() > 0);

        NamedVariableList returned(asstring, &dummy);

        CHECK(returned.GetName() == original.GetName());
        REQUIRE(returned.GetValueDirect());
        REQUIRE(original.GetValueDirect());
        CHECK(returned.GetValue() == original.GetValue());
    }
}

TEST_CASE("Access name")
{
    // Not that useful, for making sure functions are defined properly
    Leviathan::NamedVariableList list("name thing", new VariableBlock(24));

    CHECK(list.GetName() == "name thing");
    CHECK(list.GetValue().ConvertAndReturnVariable<int>() == 24);
}

TEST_CASE("NamedVariableList::ProcessDataDump data flow all defs 1", "[variable]")
{
    DummyReporter dummy;

    std::vector<std::shared_ptr<NamedVariableList>> variables;
    REQUIRE(NamedVariableList::ProcessDataDump("a = 1;", variables, &dummy));

    REQUIRE(variables.size() == 1);
    REQUIRE(variables[0]);

    CHECK(variables[0]->GetName() == "a");
    CHECK(variables[0]->GetValue().ConvertAndReturnVariable<int>() == 1);
}

TEST_CASE("NamedVariableList::ProcessDataDump data flow all defs 2", "[variable]")
{
    DummyReporter dummy;

    std::vector<std::shared_ptr<NamedVariableList>> variables;
    CHECK(!NamedVariableList::ProcessDataDump(";", variables, &dummy));

    CHECK(variables.size() == 0);
}

TEST_CASE("NamedVariableList::ProcessDataDump data flow all defs 1 variant", "[variable]")
{
    DummyReporter dummy;

    std::vector<std::shared_ptr<NamedVariableList>> variables;
    REQUIRE(NamedVariableList::ProcessDataDump("a=1;", variables, &dummy));

    REQUIRE(variables.size() == 1);
    REQUIRE(variables[0]);

    CHECK(variables[0]->GetName() == "a");
    CHECK(variables[0]->GetValue().ConvertAndReturnVariable<int>() == 1);
}

TEST_CASE(
    "NamedVariableList::ProcessDataDump data flow all p uses exception usage", "[variable]")
{
    RequireErrorReporter reporter;

    std::vector<std::shared_ptr<NamedVariableList>> variables;
    CHECK(NamedVariableList::ProcessDataDump(" = 1;", variables, &reporter));

    CHECK(variables.size() == 0);
}

TEST_CASE("NamedVariableList::ProcessDataDump data flow all p-uses 1", "[variable]")
{
    DummyReporter dummy;

    std::vector<std::shared_ptr<NamedVariableList>> variables;
    CHECK(!NamedVariableList::ProcessDataDump("thing", variables, &dummy));

    CHECK(variables.size() == 0);
}

TEST_CASE("NamedVariableList::ProcessDataDump data flow all p-uses 2", "[variable]")
{
    DummyReporter dummy;

    std::vector<std::shared_ptr<NamedVariableList>> variables;
    REQUIRE(NamedVariableList::ProcessDataDump("thing = \"stuff\";", variables, &dummy));

    REQUIRE(variables.size() == 1);
    REQUIRE(variables[0]);

    CHECK(variables[0]->GetName() == "thing");
    CHECK(variables[0]->GetValue().ConvertAndReturnVariable<std::string>() == "stuff");
}

TEST_CASE("CEF dictionary to NamedVars conversion", "[variable]")
{
    SECTION("CEF dictionary works")
    {
        CefRefPtr<CefDictionaryValue> dictionary = CefDictionaryValue::Create();
        dictionary->SetBool("primary", true);
        CHECK(dictionary->GetBool("primary") == true);
        std::vector<CefString> keys;
        REQUIRE(dictionary->GetKeys(keys));

        REQUIRE(keys.size() == 1);
        CHECK(keys[0] == "primary");
    }

    SECTION("Empty dictionary")
    {
        CefRefPtr<CefDictionaryValue> dictionary = CefDictionaryValue::Create();
        NamedVars vars;
        CEFDictionaryToNamedVars(dictionary, vars);

        CHECK(vars.GetVariableCount() == 0);
    }

    SECTION("Dictionary with one bool key")
    {
        CefRefPtr<CefDictionaryValue> dictionary = CefDictionaryValue::Create();
        dictionary->SetBool("primary", true);

        NamedVars vars;
        CEFDictionaryToNamedVars(dictionary, vars);

        CHECK(vars.GetVariableCount() == 1);
        REQUIRE(vars.GetValue("primary"));
        CHECK(vars.GetValue("primary")->ConvertAndReturnVariable<bool>() == true);
    }

    SECTION("Dictionary with a string")
    {
        constexpr auto str = "this is a test string";
        CefRefPtr<CefDictionaryValue> dictionary = CefDictionaryValue::Create();
        dictionary->SetString("str", str);

        NamedVars vars;
        CEFDictionaryToNamedVars(dictionary, vars);

        CHECK(vars.GetVariableCount() == 1);
        REQUIRE(vars.GetValue("str"));
        CHECK(vars.GetValue("str")->ConvertAndReturnVariable<std::string>() == str);
    }
}
