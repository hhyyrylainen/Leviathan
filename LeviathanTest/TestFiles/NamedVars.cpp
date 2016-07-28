#include "Common/DataStoring/NamedVars.h"

#include "ObjectFiles/ObjectFileProcessor.h"

#include "catch.hpp"
#include "../DummyLog.h"

using namespace Leviathan;
using namespace std;

TEST_CASE("NamedVars creation and value retrieve", "[variable]"){

    vector<shared_ptr<NamedVariableList>> Variables;
    Variables.push_back(shared_ptr<NamedVariableList>(new NamedVariableList("var1",
                new VariableBlock(1))));
    
    Variables.push_back(shared_ptr<NamedVariableList>(new NamedVariableList("var2",
                new VariableBlock(2))));
    
    Variables.push_back(shared_ptr<NamedVariableList>(new NamedVariableList("var3",
                new VariableBlock(3))));
    
    Variables.push_back(shared_ptr<NamedVariableList>(new NamedVariableList("var4",
                new VariableBlock(4))));
    
    Variables.push_back(shared_ptr<NamedVariableList>(new NamedVariableList("var5",
                new VariableBlock(5))));

    // create holder //
    NamedVars holder = NamedVars();

    holder.SetVec(Variables);

    // add some more values //
    holder.AddVar("var66", new VariableBlock(25));
    holder.Remove(holder.Find("var2"));

    CHECK(holder.Find("var66") < holder.GetVariableCount());
    CHECK(holder.Find("var2") >= holder.GetVariableCount());

    int checkval;
    
    REQUIRE(holder.GetValueAndConvertTo<int>("var3", checkval) == true);

    CHECK(checkval == 3);
}

TEST_CASE("NamedVars line parsing", "[variable, objectfiles]"){

    DummyReporter reporter;

    SECTION("Line 'this= not this'"){
        
        string typelinething = "this= not this";

        // creation testing //
        auto ptry = make_shared<NamedVariableList>(typelinething, &reporter);

        wstring emptystr;
        
        REQUIRE(ptry->GetValueDirect()->ConvertAndAssingToVariable<wstring>(emptystr) == true);

        CHECK(emptystr == L"not this");
    }

    SECTION("Line 'this=2'"){
        
        string typelinething = "this=2";

        auto result = shared_ptr<NamedVariableList>(new NamedVariableList(typelinething, &reporter));

        int checkval;

        REQUIRE(result->GetValueDirect()->ConvertAndAssingToVariable<int>(checkval) == true);

        CHECK(checkval == 2);
    }

    SECTION("Line 'oh=2;'"){
        
        string line = "oh=2;";
    
        auto result = make_shared<NamedVariableList>(line, &reporter);

        REQUIRE(result);

        CHECK(result->GetName() == "oh");
    }

    SECTION("Basic bracket expression"){

        
        NamedVariableList advlist("value = [first, 2]", &reporter);

        REQUIRE(advlist.GetVariableCount() == 2);

        CHECK(advlist.GetValueDirect(0)->GetBlockConst()->Type == DATABLOCK_TYPE_STRING);
    
        CHECK(advlist.GetValueDirect(1)->GetBlockConst()->Type == DATABLOCK_TYPE_INT);
    }

    SECTION("Advanced line 'Color = [[0.1], [4], [true], [\"lol\"]]'"){

        string linething = "Color = [[0.1], [4], [true], [\"lol\"]]";
        NamedVariableList advlist(linething, &reporter);

        REQUIRE(advlist.GetVariableCount() == 4);

        CHECK(advlist.GetValueDirect(0)->GetBlockConst()->Type == DATABLOCK_TYPE_FLOAT);
    
        CHECK(advlist.GetValueDirect(1)->GetBlockConst()->Type == DATABLOCK_TYPE_INT);
    
        CHECK(advlist.GetValueDirect(2)->GetBlockConst()->Type == DATABLOCK_TYPE_BOOL);
    
        CHECK(advlist.GetValueDirect(3)->GetBlockConst()->Type == DATABLOCK_TYPE_STRING);
    }
}

#ifdef SFML_PACKETS
TEST_CASE("NamedVars packet serialization", "[variable]"){

    NamedVars packettestorig;

    packettestorig.AddVar("MyVar1", new VariableBlock((string)"string_block"));
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

TEST_CASE("Specific value parsing", "[variable]"){

    DummyReporter reporter;

    SECTION("Width = 1280;"){

        NamedVariableList var("Width = 1280;", &reporter);

        REQUIRE(var.GetVariableCount() == 1);
        CHECK(var.GetCommonType() == DATABLOCK_TYPE_INT);
        CHECK(var.GetValue(0).operator int() == 1280);

        SECTION("Stringification"){
            
            CHECK(var.ToText(0, true) == "Width = [[1280]];");
            CHECK(var.ToText(1, true) == "Width: [[1280]];");


            SECTION("Parsing back from string"){

                NamedVariableList var2(var.ToText(0), &reporter);

                REQUIRE(var2.GetVariableCount() == 1);
                CHECK(var2.GetCommonType() == DATABLOCK_TYPE_INT);
                CHECK(var2.GetValue(0).operator int() == 1280);                
            }
        }
    }

    SECTION("Engine conf sample and variable loading"){

        NamedVars values("Width = 1280;\n"
            "Height = [[720]];\n"
            "Windowed = [true];\n"
            "RenderSystemName = [[[\"Open.*GL\"]]];\n", &reporter
        );
        
        int width;
        int height;
        bool window;
        string rendersystemname;

        ObjectFileProcessor::LoadValueFromNamedVars(values, "Width", width, 0);

        CHECK(width == 1280);
        
        ObjectFileProcessor::LoadValueFromNamedVars(values, "Height", height,
            0);

        CHECK(height == 720);
        
        ObjectFileProcessor::LoadValueFromNamedVars(values, "Windowed", window,
            false);

        CHECK(window == true);

        ObjectFileProcessor::LoadValueFromNamedVars(values,
            "RenderSystemName", rendersystemname, string(""));

        CHECK(rendersystemname == "Open.*GL");
    }

    SECTION("DataStore things"){

        NamedVariableList var("StartCount = [[1]];", &reporter);

        REQUIRE(var.GetVariableCount() == 1);
        CHECK(var.GetCommonType() == DATABLOCK_TYPE_INT);
        CHECK(var.GetValue().operator int() == 1);

        var = NamedVariableList("StartCount", new VariableBlock(
                new IntBlock(var.GetValue().operator int() + 1)));

        CHECK(var.GetCommonType() == DATABLOCK_TYPE_INT);
        CHECK(var.GetValue().operator int() == 2);
        CHECK(var.ToText(0, true) == "StartCount = [[2]];");
        CHECK(var.ToText(0, false) == "StartCount = 2;");
        CHECK(var.ToText(1, false) == "StartCount: 2;");


        var = NamedVariableList("StartCount", new VariableBlock(
                new IntBlock(var.GetValue().operator int() + 1)));

        CHECK(var.GetCommonType() == DATABLOCK_TYPE_INT);
        CHECK(var.GetValue().operator int() == 3);
        CHECK(var.ToText(0, true) == "StartCount = [[3]];");
        CHECK(var.ToText(0, false) == "StartCount = 3;");

    }
}

TEST_CASE("Converting empty string blocks", "[variable, datablock]"){


    NamedVariableList empty("name", new StringBlock(new string()));

    CHECK(empty.GetValue(0).operator int() == 0);
    CHECK(empty.GetValue(0).operator char() == 0);
    CHECK(empty.GetValue(0).operator float() == 0.f);
    CHECK(empty.GetValue(0).operator double() == 0.0);
    CHECK(empty.GetValue(0).operator string() == "");
    CHECK(empty.GetValue(0).operator wstring() == L"");
    CHECK(empty.GetValue(0).operator bool() == false);
    
}

TEST_CASE("Allow missing ending';'", "[variable, datablock]") {

    DummyReporter dummy;
    NamedVariableList testlist("name = 13.5", &dummy);

    REQUIRE(testlist.IsValid());
    REQUIRE(testlist.GetVariableCount() == 1);

    REQUIRE(testlist.GetValue().IsConversionAllowedNonPtr<float>());

    CHECK(static_cast<float>(testlist.GetValue()) == 13.5f);
}

TEST_CASE("Verify equals operator works", "[variable, datablock]") {

    SECTION("Directly with VariableBlocks") {

        CHECK(VariableBlock(nullptr) != VariableBlock(nullptr));

        CHECK(VariableBlock(new IntBlock(2)) != VariableBlock(nullptr));
        CHECK(VariableBlock(new IntBlock(2)) != VariableBlock(new IntBlock(3)));
        CHECK(VariableBlock(new IntBlock(2)) != VariableBlock(new StringBlock("3")));

        CHECK(VariableBlock(new IntBlock(2)) == VariableBlock(new IntBlock(2)));
        CHECK(!(VariableBlock(new IntBlock(2)) != VariableBlock(new IntBlock(2))));

        CHECK(VariableBlock(std::string("how")) == VariableBlock(std::string("how")));
        CHECK(VariableBlock(std::string("how")) != VariableBlock(std::string("how2")));

    }

    SECTION("Through NamedVariableList") {

        CHECK(NamedVariableList("val") == NamedVariableList("val"));
        CHECK(!(NamedVariableList("val") != NamedVariableList("val")));

        CHECK(NamedVariableList("val", new VariableBlock(nullptr)) != 
            NamedVariableList("val", new VariableBlock(nullptr)));

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

TEST_CASE("Parsing values back from ToText", "[variable, datablock]") {

    DummyReporter dummy;

    SECTION("Empty value into empty string") {

        NamedVariableList original("val");

        const auto asstring = original.ToText(0, false);

        CHECK(asstring.length() > 0);

        NamedVariableList returned(asstring, &dummy);

        CHECK(returned.GetName() == original.GetName());
        CHECK(returned.GetValueDirect() != original.GetValueDirect());
        CHECK(returned.GetValueDirect());
    }

    SECTION("empty string") {

        NamedVariableList original("val", new VariableBlock(std::string("")));

        const auto asstring = original.ToText(0, true);

        CHECK(asstring.length() > 0);

        NamedVariableList returned(asstring, &dummy);

        CHECK(returned.GetName() == original.GetName());
        REQUIRE(returned.GetValueDirect());
        REQUIRE(original.GetValueDirect());
        CHECK(returned.GetValue() == original.GetValue());
    }

    SECTION("a simple string") {

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
