#include "Iterators/StringIterator.h"
#include "utf8.h"

#include "boost/assign.hpp"

#include "catch.hpp"

using namespace Leviathan;

TEST_CASE("StringIterator get functions", "[string, objectfile]"){

	StringIterator itr((string*)NULL);

    //itr.SetDebugMode(true);

    SECTION("Getting strings in quotes"){

     	itr.ReInit(L" get \" this stuff in here\\\" which has 'stuff' \"_ and not this");

        auto results = itr.GetStringInQuotes<wstring>(QUOTETYPE_DOUBLEQUOTES);

        CHECK(*results == L" this stuff in here\\\" which has 'stuff' ");

        // We should now be on "_" //
        CHECK(itr.GetCharacter() == L'_');
    }

    SECTION("Character ignored and get nothing"){

        itr.ReInit(";quick testcase for \\;not getting anything!");

        auto results = itr.GetUntilNextCharacterOrNothing<string>(L';');

        REQUIRE(results == nullptr);
    }

    SECTION("Skip unnormal first whitespace"){

        itr.ReInit("		teesti_ess y");

        auto results = itr.GetNextCharacterSequence<string>(UNNORMALCHARACTER_TYPE_LOWCODES |
            UNNORMALCHARACTER_TYPE_WHITESPACE);

        REQUIRE(results != nullptr);
        
        CHECK(*results == "teesti_ess");
    }

    SECTION("Object first line correct letter"){

        SECTION("String"){
            itr.ReInit(" o object type");

            auto results = itr.GetNextCharacterSequence<string>(UNNORMALCHARACTER_TYPE_WHITESPACE |
                UNNORMALCHARACTER_TYPE_CONTROLCHARACTERS);
        
            REQUIRE(results != nullptr);
            CHECK(*results == "o");
        }

        SECTION("Wstring"){
            
            itr.ReInit(L" o object type");

            auto results = itr.GetNextCharacterSequence<wstring>(
                UNNORMALCHARACTER_TYPE_WHITESPACE | UNNORMALCHARACTER_TYPE_CONTROLCHARACTERS);
        
            REQUIRE(results != nullptr);
            CHECK(*results == L"o");
        }
    }

    
}

TEST_CASE("StringIterator UTF8 correctness", "[string, objectfile, utf8]"){

    // Test UTF8 string handling //

	std::vector<int> unicodeholder = boost::assign::list_of(0x00E4)('_')(0x0503)(0x04E8)(0x0A06)
        (0x1304)(0xAC93)(0x299D);

	string toutf8;

	utf8::utf32to8(unicodeholder.begin(), unicodeholder.end(), back_inserter(toutf8));

	wstring resultuni16;

	utf8::utf8to16(toutf8.begin(), toutf8.end(), back_inserter(resultuni16));

	
	StringIterator itr(new UTF8DataIterator(toutf8), true);

	auto shouldbethesame = itr.GetUntilNextCharacterOrAll<string>('a');

    CHECK(*shouldbethesame == toutf8);
	
	itr.ReInit(new UTF8DataIterator("My Super nice \\= unicode is this : \""+toutf8+"\""), true);
	itr.GetUntilEqualityAssignment<string>(EQUALITYCHARACTER_TYPE_ALL);

	// Now get the UTF8 sequence //
	auto utf8encoded = itr.GetStringInQuotes<string>(QUOTETYPE_DOUBLEQUOTES);

	// Convert to utf 16 and compare //
    REQUIRE(utf8encoded != nullptr);

    wstring cvrtsy;
    
    utf8::utf8to16(utf8encoded->begin(), utf8encoded->end(), back_inserter(cvrtsy));

    CHECK(cvrtsy == resultuni16);
}
