#include "Iterators/StringIterator.h"
#include "utf8.h"

#include "catch.hpp"
#include "Logger.h"

using namespace Leviathan;
using namespace std;

TEST_CASE("StringIterator get functions", "[string, objectfile]"){

    // For outputting debug info //
    Logger log("Test/TestLog.txt");

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

    SECTION("Whitespace and control characters stop"){

        itr.ReInit("get-this nice_prefix[but not this!");

        auto results = itr.GetNextCharacterSequence<string>(UNNORMALCHARACTER_TYPE_WHITESPACE |
            UNNORMALCHARACTER_TYPE_CONTROLCHARACTERS);

        REQUIRE(results != nullptr);
        CHECK(*results == "get-this");

        results = itr.GetNextCharacterSequence<string>(UNNORMALCHARACTER_TYPE_WHITESPACE |
            UNNORMALCHARACTER_TYPE_CONTROLCHARACTERS);

        REQUIRE(results != nullptr);
        CHECK(*results == "nice_prefix");
    }

    SECTION("Getting decimal separator numbers"){

        itr.ReInit("aib val: = 243.12al toi() a 2456,12.5");

        auto results = itr.GetNextNumber<string>(DECIMALSEPARATORTYPE_DOT);

        REQUIRE(results != nullptr);
        CHECK(*results == "243.12");

        results = itr.GetNextNumber<string>(DECIMALSEPARATORTYPE_DOT);

        REQUIRE(results != nullptr);
        CHECK(*results == "2456");

        results = itr.GetNextNumber<string>(DECIMALSEPARATORTYPE_DOT);

        REQUIRE(results != nullptr);
        CHECK(*results == "12.5");
    }

    SECTION("NamedVars style parsing"){

        SECTION("Name getting with extra ':'"){

            itr.ReInit("	aib val: = 243.12al toi() a 2456,12.5");

            auto results = itr.GetUntilEqualityAssignment<string>(EQUALITYCHARACTER_TYPE_EQUALITY);

            REQUIRE(results != nullptr);
            CHECK(*results == "aib val:");
        }
        
        SECTION("StartCount test data"){

            itr.ReInit(L"StartCount = [[245]];");

            auto results = itr.GetUntilEqualityAssignment<wstring>(
                EQUALITYCHARACTER_TYPE_EQUALITY);

            REQUIRE(results != nullptr);
            CHECK(*results == L"StartCount");

            itr.SkipWhiteSpace();

            results = itr.GetUntilNextCharacterOrAll<wstring>(L';');

            REQUIRE(results != nullptr);
            CHECK(*results == L"[[245]]");
        }

        SECTION("Claustrofobic values 'oh=2;'"){

            itr.ReInit(L"oh=2;");

            auto results = itr.GetUntilEqualityAssignment<wstring>(EQUALITYCHARACTER_TYPE_ALL);

            REQUIRE(results != nullptr);
            CHECK(*results == L"oh");

            itr.SkipWhiteSpace(SPECIAL_ITERATOR_FILEHANDLING);

            results = itr.GetUntilNextCharacterOrNothing<wstring>(';');

            REQUIRE(results != nullptr);
            CHECK(*results == L"2");
        }
    }

    SECTION("Until character properly ignores after \\"){

        itr.ReInit(" adis told as\\; this still ; and no this");

        auto results = itr.GetUntilNextCharacterOrNothing<string>(';');

        REQUIRE(results != nullptr);
        CHECK(*results == " adis told as\\; this still ");
    }

    SECTION("Getting data near control characters"){

        itr.ReInit("not][ this<out>");

        auto results = itr.GetNextCharacterSequence<string>(
            UNNORMALCHARACTER_TYPE_CONTROLCHARACTERS);

        REQUIRE(results != nullptr);
        CHECK(*results == "not");

        results = itr.GetNextCharacterSequence<string>(
            UNNORMALCHARACTER_TYPE_CONTROLCHARACTERS);

        REQUIRE(results != nullptr);
        CHECK(*results == " this");
    }

    SECTION("Quoted word with target character after"){

        itr.ReInit("\"JellyCube\";");
            
        auto results = itr.GetUntilNextCharacterOrAll<string>(';');

        REQUIRE(results != nullptr);
        CHECK(*results == "\"JellyCube\"");
    }

    SECTION("Comments with newlines"){

        itr.ReInit("asdf // This is a comment! //\n 25.44 /*2 .*/\n12\n\n"
            "// 1\n  a /*42.1*/12");

        auto results = itr.GetNextNumber<string>(DECIMALSEPARATORTYPE_DOT,
            SPECIAL_ITERATOR_HANDLECOMMENTS_ASSTRING);

        REQUIRE(results != nullptr);
        CHECK(*results == "25.44");

        results = itr.GetNextNumber<string>(DECIMALSEPARATORTYPE_DOT,
            SPECIAL_ITERATOR_HANDLECOMMENTS_ASSTRING);

        REQUIRE(results != nullptr);
        CHECK(*results == "12");
            
        results = itr.GetNextNumber<string>(DECIMALSEPARATORTYPE_DOT,
            SPECIAL_ITERATOR_HANDLECOMMENTS_ASSTRING);

        REQUIRE(results != nullptr);
        CHECK(*results == "12");
    }

    SECTION("Multiline stopping at line end"){

        itr.ReInit(L"Don\\'t get anything from here\n42, but here it is1\n"
            "4 get until this\n and not this[as\n;] \"how it cu\nts\"");

        auto results = itr.GetNextNumber<wstring>(DECIMALSEPARATORTYPE_NONE,
            SPECIAL_ITERATOR_ONNEWLINE_STOP);

        CHECK(results == nullptr);

        results = itr.GetUntilNextCharacterOrNothing<wstring>(',');

        REQUIRE(results != nullptr);
        CHECK(*results == L"42");
            
        results = itr.GetNextNumber<wstring>(DECIMALSEPARATORTYPE_NONE,
            SPECIAL_ITERATOR_ONNEWLINE_STOP);

        REQUIRE(results != nullptr);
        CHECK(*results == L"1");

        results = itr.GetNextCharacterSequence<wstring>(
            UNNORMALCHARACTER_TYPE_CONTROLCHARACTERS, SPECIAL_ITERATOR_ONNEWLINE_STOP);

        REQUIRE(results != nullptr);
        CHECK(*results == L"4 get until this");

        results = itr.GetUntilNextCharacterOrNothing<wstring>('[');

        CHECK(results != nullptr);

        results = itr.GetUntilNextCharacterOrNothing<wstring>(';',
            SPECIAL_ITERATOR_ONNEWLINE_STOP);

        CHECK(results == nullptr);

        results = itr.GetStringInQuotes<wstring>(QUOTETYPE_BOTH,
            SPECIAL_ITERATOR_ONNEWLINE_STOP);

        REQUIRE(results != nullptr);
        CHECK(*results == L"how it cu");            
    }

    SECTION("Getting until line end with escaped newline and windows style line change"){

        itr.ReInit(L"This is my line \\\r that has some things\r\n that are cut off");

        auto results = itr.GetUntilLineEnd<wstring>();

        REQUIRE(results != nullptr);
        CHECK(*results == L"This is my line \\\r that has some things");
    }

    SECTION("Getting until line end with proper style"){

        itr.ReInit(L"This is another line\nwith the right separator\nlast thing...");

        auto results = itr.GetUntilLineEnd<wstring>();

        REQUIRE(results != nullptr);
        CHECK(*results == L"This is another line");

        results = itr.GetUntilLineEnd<wstring>();

        REQUIRE(results != nullptr);
        CHECK(*results == L"with the right separator");

        results = itr.GetUntilLineEnd<wstring>();

        REQUIRE(results != nullptr);
        CHECK(*results == L"last thing...");
    }

    SECTION("Getting until string"){

        itr.ReInit(L"Get until ('bird') the word bird to remove junkwords");

        auto results = itr.GetUntilCharacterSequence<wstring>(L"bird");

        REQUIRE(results != nullptr);
        CHECK(*results == L"Get until ('bird') the word ");

        itr.MoveToNext();

        results = itr.GetUntilCharacterSequence<wstring>(L"word");

        REQUIRE(results != nullptr);
        CHECK(*results == L" to remove junk");
    }
}

TEST_CASE("StringIterator UTF8 correctness", "[string, objectfile, utf8]"){

    // Test UTF8 string handling //

    std::vector<int> unicodeholder = { 0x00E4, '_', 0x0503, 0x04E8, 0x0A06, 0x1304, 0xAC93, 0x299D };

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

TEST_CASE("StringIterator bracket handling", "[string, objectfile]"){
    
    StringIterator itr;
    std::unique_ptr<std::string> result;

    SECTION("Basic string"){
        
        itr.ReInit("my first [bracket value test] that is]] nice");
        result = itr.GetStringInBracketsRecursive<string>();

        REQUIRE(result);
        CHECK(*result == "bracket value test");

        itr.ReInit("[Another test]");

        result = itr.GetStringInBracketsRecursive<string>();

        REQUIRE(result);
        CHECK(*result == "Another test");
    }

    SECTION("Return NULL when missing end"){
        
        itr.ReInit("[Another test");

        result = itr.GetStringInBracketsRecursive<string>();

        REQUIRE_FALSE(result);
    }

    SECTION("Nesting all begin in a row"){

        itr.ReInit("[[[Nice deeply ] nested] brackets]");

        result = itr.GetStringInBracketsRecursive<string>();

        REQUIRE(result);
        CHECK(*result == "[[Nice deeply ] nested] brackets");
    }

    SECTION("Line end is ignored without flag"){
        
        itr.ReInit("[[[Nice deeply ]\n nested] brackets]");

        result = itr.GetStringInBracketsRecursive<string>();

        REQUIRE(result);
        CHECK(*result == "[[Nice deeply ]\n nested] brackets");
    }

    SECTION("Invalid end with line end when the flag specified"){
        
        itr.ReInit("[[[Nice deeply ]\n nested] brackets]");

        result = itr.GetStringInBracketsRecursive<string>(SPECIAL_ITERATOR_ONNEWLINE_STOP);

        REQUIRE_FALSE(result);
    }

    SECTION("Actual normal use data"){
        
        itr.ReInit("[[actual, usecases, [[things, other], final], last]]");

        result = itr.GetStringInBracketsRecursive<string>();

        REQUIRE(result);
        CHECK(*result == "[actual, usecases, [[things, other], final], last]");

        itr.ReInit(*result);

        result = itr.GetStringInBracketsRecursive<string>();

        REQUIRE(result);
        CHECK(*result == "actual, usecases, [[things, other], final], last");

        itr.ReInit(*result);

        result = itr.GetStringInBracketsRecursive<string>();

        REQUIRE(result);
        CHECK(*result == "[things, other], final");
    }
}
