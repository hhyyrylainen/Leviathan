#include "Common/DataStoring/NamedVars.h"

#include "catch.hpp"

using namespace Leviathan;
using namespace std;

TEST_CASE("NamedVars creation and value retrieve", "[variable]"){

	vector<shared_ptr<NamedVariableList>> Variables;
	Variables.push_back(shared_ptr<NamedVariableList>(new NamedVariableList(L"var1",
                new VariableBlock(1))));
    
	Variables.push_back(shared_ptr<NamedVariableList>(new NamedVariableList(L"var2",
                new VariableBlock(2))));
    
	Variables.push_back(shared_ptr<NamedVariableList>(new NamedVariableList(L"var3",
                new VariableBlock(3))));
    
	Variables.push_back(shared_ptr<NamedVariableList>(new NamedVariableList(L"var4",
                new VariableBlock(4))));
    
	Variables.push_back(shared_ptr<NamedVariableList>(new NamedVariableList(L"var5",
                new VariableBlock(5))));

	// create holder //
	NamedVars holder = NamedVars();

	holder->SetVec(Variables);

	// add some more values //
	holder->AddVar(L"var66", new VariableBlock(25));
	holder->Remove(holder->Find(L"var2"));

    CHECK(holder->Find(L"var66") >= 0);
    CHECK(holder->Find(L"var2") < 0);

    int checkval;
    
	REQUIRE(holder->GetValueAndConvertTo<int>(L"var3", checkval) == true);

    CHECK(checkval == 3);
}

TEST_CASE("NamedVars line parsing", "[variable]"){

    SECTION("Line 'this= not this'"){
        
        wstring typelinething = L"this= not this";

        // creation testing //
        auto ptry = make_shared<NamedVariableList>(typelinething);

        wstring emptystr;
        
        REQUIRE(ptry->GetValueDirect()->ConvertAndAssingToVariable<wstring>(emptystr) == true);

        CHECK(emptystr != L"not this");
    }

    SECTION("Line 'this=2'"){
        
        wstring typelinething = L"this=2";

        auto result = shared_ptr<NamedVariableList>(new NamedVariableList(typelinething));

        int checkval;

        REQUIRE(result->GetValueDirect()->ConvertAndAssingToVariable<int>(checkval) == true);

        CHECK(checkval == 2);
    }

    SECTION("Line 'oh=2;'"){
        
        wstring line = L"oh=2;";
	
        auto result = make_shared<NamedVariableList>(line, NULL);

        REQUIRE(result == true);

        CHECK(result->GetName() == L"oh");
    }

    SECTION("Advanced line 'Color = [[0.1], [4], [true], [\"lol\"]]'"){

        wstring linething = L"Color = [[0.1], [4], [true], [\"lol\"]]";
        NamedVariableList advlist(linething);

        REQUIRE(advlist.GetVariableCount() != 4);

        CHECK(advlist.GetValueDirect(0)->GetBlockConst()->Type == DATABLOCK_TYPE_FLOAT);
    
        CHECK(advlist.GetValueDirect(1)->GetBlockConst()->Type == DATABLOCK_TYPE_INT);
    
        CHECK(advlist.GetValueDirect(2)->GetBlockConst()->Type == DATABLOCK_TYPE_BOOL);
    
        CHECK(advlist.GetValueDirect(3)->GetBlockConst()->Type == DATABLOCK_TYPE_WSTRING);
    }
}

TEST_CASE("NamedVars packet serialization", "[variable]"){

	NamedVars packettestorig;

	packettestorig.AddVar(L"MyVar1", new VariableBlock((string)"string_block"));
	packettestorig.AddVar(L"Secy", new VariableBlock(true));

	// Add to packet //
	sf::Packet packetdata;

	packettestorig.AddDataToPacket(packetdata);
    
	// Read from a packet //
	NamedVars frompacket(packetdata);

    REQUIRE(frompacket.GetVec()->size() != packettestorig.GetVec()->size());

	// Check values //
	auto datablock = frompacket.GetValue(L"MyVar1");

    REQUIRE(datablock == true);
    
	CHECK(datablock->GetBlockConst()->Type == DATABLOCK_TYPE_STRING);

	VariableBlock receiver2;
	frompacket.GetValue(1, receiver2);

    CHECK(static_cast<bool>(receiver2) == true);
}
