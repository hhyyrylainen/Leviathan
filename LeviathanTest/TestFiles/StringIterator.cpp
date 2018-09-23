#include "Iterators/StringIterator.h"
#include "../PartialEngine.h"
#include "utf8.h"

#include "catch.hpp"

using namespace Leviathan;
using namespace Leviathan::Test;

TEST_CASE("StringIterator basic utf8 pointer", "[string]")
{
    constexpr auto* testdata = "my first test string";

    SECTION("Direct use")
    {
        UTF8PointerDataIterator data{
            testdata, testdata + std::char_traits<char>::length(testdata)};

        CHECK(data.CurrentIteratorPosition() == 0);
        CHECK(data.GetCurrentLineNumber() == 1);
        CHECK(data.GetLastValidIteratorPosition() ==
              std::char_traits<char>::length(testdata) - 1);

        data.MoveToNextCharacter();
        CHECK(data.CurrentIteratorPosition() == 1);
        CHECK(data.GetLastValidIteratorPosition() ==
              std::char_traits<char>::length(testdata) - 1);
        CHECK(data.IsPositionValid());

        while(data.IsPositionValid()) {

            REQUIRE_NOTHROW(data.MoveToNextCharacter());
        }
    }

    SECTION("In iterator")
    {
        StringIterator itr(std::make_unique<UTF8PointerDataIterator>(
            testdata, testdata + std::char_traits<char>::length(testdata)));

        SECTION("Entire string")
        {
            auto str = itr.GetUntilEnd<std::string>();
            CHECK(*str == testdata);
        }

        SECTION("Until some character")
        {
            auto str = itr.GetUntilNextCharacterOrAll<std::string>('t');
            CHECK(*str == "my firs");
        }
    }
}

TEST_CASE("StringIterator::GetStringInQuotes", "[string]")
{
    TestLogger log("Test/StringIteratorTest.txt");

    StringIterator itr;

    SECTION("Works on empty input")
    {
        itr.ReInit("");
        CHECK(!itr.GetStringInQuotes<std::string>(QUOTETYPE_BOTH));
    }

    SECTION("Works on input with no quotes")
    {
        itr.ReInit("here is a string with no quotes");
        CHECK(!itr.GetStringInQuotes<std::string>(QUOTETYPE_BOTH));
    }

    SECTION("End of string is handled properly")
    {
        SECTION("non-terminating quotes")
        {
            itr.ReInit("here is a string with nothing to 'find");
            CHECK(!itr.GetStringInQuotes<std::string>(QUOTETYPE_BOTH));
        }

        SECTION("ending as the last thing in a string")
        {
            itr.ReInit("here is a string with nothing to 'find'");
            auto result = itr.GetStringInQuotes<std::string>(QUOTETYPE_BOTH);
            REQUIRE(result);
            CHECK(*result == "find");
        }
    }

    SECTION("QUOTETYPE works")
    {
        SECTION("Single quote ignores double quotes")
        {
            itr.ReInit("this is a 'string' with both quote \"types\"");
            auto result = itr.GetStringInQuotes<std::string>(QUOTETYPE_SINGLEQUOTES);
            REQUIRE(result);
            CHECK(*result == "string");

            CHECK(!itr.GetStringInQuotes<std::string>(QUOTETYPE_SINGLEQUOTES));
        }

        SECTION("Double quote ignores single quotes")
        {
            itr.ReInit("this is a 'string' with both quote \"types\"");
            auto result = itr.GetStringInQuotes<std::string>(QUOTETYPE_DOUBLEQUOTES);
            REQUIRE(result);
            CHECK(*result == "types");
        }

        SECTION("Both gets strings of both types")
        {
            itr.ReInit("this is a 'string' with both quote \"types\"");
            auto result = itr.GetStringInQuotes<std::string>(QUOTETYPE_BOTH);
            REQUIRE(result);
            CHECK(*result == "string");

            result = itr.GetStringInQuotes<std::string>(QUOTETYPE_BOTH);
            REQUIRE(result);
            CHECK(*result == "types");
        }
    }

    SECTION("Works on multiline input")
    {
        itr.ReInit("this is a multiline\nstring that has quotes 'on the next line'");
        auto result = itr.GetStringInQuotes<std::string>(QUOTETYPE_BOTH);
        REQUIRE(result);
        CHECK(*result == "on the next line");
    }

    SECTION("Escaped quotes are ignored")
    {
        SECTION("for starting")
        {
            itr.ReInit("this is a \\'string\\' with 'quotes'");
            auto result = itr.GetStringInQuotes<std::string>(
                QUOTETYPE_BOTH, SPECIAL_ITERATOR_ONNEWLINE_STOP);
            REQUIRE(result);
            CHECK(*result == "quotes");
        }

        SECTION("for ending")
        {
            itr.ReInit("this is a 'string\\' with 'quotes'");
            auto result = itr.GetStringInQuotes<std::string>(
                QUOTETYPE_BOTH, SPECIAL_ITERATOR_ONNEWLINE_STOP);
            REQUIRE(result);
            CHECK(*result == "string\\' with ");
        }
    }

    SECTION("Special flag stop on newline works")
    {
        SECTION("No quotes before newline")
        {
            itr.ReInit("this is a multiline\nstring that has quotes 'on the next line'");
            CHECK(!itr.GetStringInQuotes<std::string>(
                QUOTETYPE_BOTH, SPECIAL_ITERATOR_ONNEWLINE_STOP));
        }

        SECTION("Inprogress string is cut")
        {
            itr.ReInit("this is a 'multiline\nstring' that has quotes");
            auto result = itr.GetStringInQuotes<std::string>(
                QUOTETYPE_BOTH, SPECIAL_ITERATOR_ONNEWLINE_STOP);
            REQUIRE(result);
            CHECK(*result == "multiline");
        }

        SECTION(
            "Inprogress string is cut and excess quotes before the line change are removed")
        {
            SECTION("Double quotes inside single quotes")
            {
                itr.ReInit("this is a 'multiline\"\nstring\"' that has quotes");
                auto result = itr.GetStringInQuotes<std::string>(
                    QUOTETYPE_BOTH, SPECIAL_ITERATOR_ONNEWLINE_STOP);
                REQUIRE(result);
                CHECK(*result == "multiline");
            }

            SECTION("Single quotes inside double quotes")
            {
                itr.ReInit("this is a \"multiline'\nstring'\" that has quotes");
                auto result = itr.GetStringInQuotes<std::string>(
                    QUOTETYPE_BOTH, SPECIAL_ITERATOR_ONNEWLINE_STOP);
                REQUIRE(result);
                CHECK(*result == "multiline");
            }

            SECTION("With different types cutting isn't done")
            {
                SECTION("Double quotes inside single quotes")
                {
                    itr.ReInit("this is a 'multiline\"\nstring\"' that has quotes");
                    auto result = itr.GetStringInQuotes<std::string>(
                        QUOTETYPE_SINGLEQUOTES, SPECIAL_ITERATOR_ONNEWLINE_STOP);
                    REQUIRE(result);
                    CHECK(*result == "multiline\"");
                }

                SECTION("Single quotes inside double quotes")
                {
                    itr.ReInit("this is a \"multiline'\nstring'\" that has quotes");
                    auto result = itr.GetStringInQuotes<std::string>(
                        QUOTETYPE_DOUBLEQUOTES, SPECIAL_ITERATOR_ONNEWLINE_STOP);
                    REQUIRE(result);
                    CHECK(*result == "multiline'");
                }
            }
        }
    }
}

TEST_CASE("StringIterator get functions", "[string][objectfile]")
{
    // For outputting debug info //
    TestLogger log("Test/StringIteratorTest.txt");

    StringIterator itr((std::string*)NULL);

    // itr.SetDebugMode(true);

    SECTION("Getting strings in quotes")
    {
        itr.ReInit(L" get \" this stuff in here\\\" which has 'stuff' \"_ and not this");

        auto results = itr.GetStringInQuotes<std::wstring>(QUOTETYPE_DOUBLEQUOTES);

        CHECK(*results == L" this stuff in here\\\" which has 'stuff' ");

        // We should now be on "_" //
        CHECK(itr.GetCharacter() == L'_');
    }

    SECTION("Character ignored and get nothing")
    {
        itr.ReInit(";quick testcase for \\;not getting anything!");

        auto results = itr.GetUntilNextCharacterOrNothing<std::string>(L';');

        REQUIRE(results == nullptr);
    }

    SECTION("Skip unnormal first whitespace")
    {
        itr.ReInit("		teesti_ess y");

        auto results = itr.GetNextCharacterSequence<std::string>(
            UNNORMALCHARACTER_TYPE_LOWCODES | UNNORMALCHARACTER_TYPE_WHITESPACE);

        REQUIRE(results != nullptr);
        CHECK(*results == "teesti_ess");
    }

    SECTION("Object first line correct letter")
    {
        SECTION("String")
        {
            itr.ReInit(" o object type");

            auto results = itr.GetNextCharacterSequence<std::string>(
                UNNORMALCHARACTER_TYPE_WHITESPACE | UNNORMALCHARACTER_TYPE_CONTROLCHARACTERS);

            REQUIRE(results != nullptr);
            CHECK(*results == "o");
        }

        SECTION("Wstring")
        {
            itr.ReInit(L" o object type");

            auto results = itr.GetNextCharacterSequence<std::wstring>(
                UNNORMALCHARACTER_TYPE_WHITESPACE | UNNORMALCHARACTER_TYPE_CONTROLCHARACTERS);

            REQUIRE(results != nullptr);
            CHECK(*results == L"o");
        }
    }

    SECTION("Whitespace and control characters stop")
    {
        SECTION("Underscores and ['s")
        {
            itr.ReInit("get-this nice_prefix[but not this!");

            auto results = itr.GetNextCharacterSequence<std::string>(
                UNNORMALCHARACTER_TYPE_WHITESPACE | UNNORMALCHARACTER_TYPE_CONTROLCHARACTERS);

            REQUIRE(results != nullptr);
            CHECK(*results == "get-this");

            results = itr.GetNextCharacterSequence<std::string>(
                UNNORMALCHARACTER_TYPE_WHITESPACE | UNNORMALCHARACTER_TYPE_CONTROLCHARACTERS);

            REQUIRE(results != nullptr);
            CHECK(*results == "nice_prefix");
        }

        SECTION("= as a control character")
        {
            itr.ReInit("cmd=\"stuff\"");

            auto results = itr.GetNextCharacterSequence<std::string>(
                UNNORMALCHARACTER_TYPE_WHITESPACE | UNNORMALCHARACTER_TYPE_CONTROLCHARACTERS);

            REQUIRE(results != nullptr);
            CHECK(*results == "cmd");

            results = itr.GetStringInQuotes<std::string>(QUOTETYPE_BOTH);

            REQUIRE(results != nullptr);
            CHECK(*results == "stuff");
        }
    }

    SECTION("Getting decimal separator numbers")
    {
        itr.ReInit("aib val: = 243.12al toi() a 2456,12.5");

        auto results = itr.GetNextNumber<std::string>(DECIMALSEPARATORTYPE_DOT);

        REQUIRE(results != nullptr);
        CHECK(*results == "243.12");

        results = itr.GetNextNumber<std::string>(DECIMALSEPARATORTYPE_DOT);

        REQUIRE(results != nullptr);
        CHECK(*results == "2456");

        results = itr.GetNextNumber<std::string>(DECIMALSEPARATORTYPE_DOT);

        REQUIRE(results != nullptr);
        CHECK(*results == "12.5");
    }

    SECTION("NamedVars style parsing")
    {
        SECTION("Name getting with extra ':'")
        {
            itr.ReInit("	aib val: = 243.12al toi() a 2456,12.5");

            auto results =
                itr.GetUntilEqualityAssignment<std::string>(EQUALITYCHARACTER_TYPE_EQUALITY);

            REQUIRE(results != nullptr);
            CHECK(*results == "aib val:");
        }

        SECTION("StartCount test data")
        {
            itr.ReInit(L"StartCount = [[245]];");

            auto results =
                itr.GetUntilEqualityAssignment<std::wstring>(EQUALITYCHARACTER_TYPE_EQUALITY);

            REQUIRE(results != nullptr);
            CHECK(*results == L"StartCount");

            itr.SkipWhiteSpace();

            results = itr.GetUntilNextCharacterOrAll<std::wstring>(L';');

            REQUIRE(results != nullptr);
            CHECK(*results == L"[[245]]");
        }

        SECTION("Claustrofobic values 'oh=2;'")
        {
            itr.ReInit(L"oh=2;");

            auto results =
                itr.GetUntilEqualityAssignment<std::wstring>(EQUALITYCHARACTER_TYPE_ALL);

            REQUIRE(results != nullptr);
            CHECK(*results == L"oh");

            itr.SkipWhiteSpace(SPECIAL_ITERATOR_FILEHANDLING);

            results = itr.GetUntilNextCharacterOrNothing<std::wstring>(';');

            REQUIRE(results != nullptr);
            CHECK(*results == L"2");
        }
    }

    SECTION("Until character properly ignores after \\")
    {
        itr.ReInit(" adis told as\\; this still ; and no this");

        auto results = itr.GetUntilNextCharacterOrNothing<std::string>(';');

        REQUIRE(results != nullptr);
        CHECK(*results == " adis told as\\; this still ");
    }

    SECTION("Getting data near control characters")
    {
        itr.ReInit("not][ this<out>");

        auto results = itr.GetNextCharacterSequence<std::string>(
            UNNORMALCHARACTER_TYPE_CONTROLCHARACTERS);

        REQUIRE(results != nullptr);
        CHECK(*results == "not");

        results = itr.GetNextCharacterSequence<std::string>(
            UNNORMALCHARACTER_TYPE_CONTROLCHARACTERS);

        REQUIRE(results != nullptr);
        CHECK(*results == " this");
    }

    SECTION("Quoted word with target character after")
    {
        itr.ReInit("\"JellyCube\";");

        auto results = itr.GetUntilNextCharacterOrAll<std::string>(';');

        REQUIRE(results != nullptr);
        CHECK(*results == "\"JellyCube\"");
    }

    SECTION("Comments with newlines")
    {
        itr.ReInit("asdf // This is a comment! //\n 25.44 /*2 .*/\n12\n\n"
                   "// 1\n  a /*42.1*/12");

        auto results = itr.GetNextNumber<std::string>(
            DECIMALSEPARATORTYPE_DOT, SPECIAL_ITERATOR_HANDLECOMMENTS_ASSTRING);

        REQUIRE(results != nullptr);
        CHECK(*results == "25.44");

        results = itr.GetNextNumber<std::string>(
            DECIMALSEPARATORTYPE_DOT, SPECIAL_ITERATOR_HANDLECOMMENTS_ASSTRING);

        REQUIRE(results != nullptr);
        CHECK(*results == "12");

        results = itr.GetNextNumber<std::string>(
            DECIMALSEPARATORTYPE_DOT, SPECIAL_ITERATOR_HANDLECOMMENTS_ASSTRING);

        REQUIRE(results != nullptr);
        CHECK(*results == "12");
    }

    SECTION("Multiline stopping at line end")
    {
        itr.ReInit(L"Don\\'t get anything from here\n42, but here it is1\n"
                   "4 get until this\n and not this[as\n;] \"how it cu\nts\"");

        auto results = itr.GetNextNumber<std::wstring>(
            DECIMALSEPARATORTYPE_NONE, SPECIAL_ITERATOR_ONNEWLINE_STOP);

        CHECK(results == nullptr);

        results = itr.GetUntilNextCharacterOrNothing<std::wstring>(',');

        REQUIRE(results != nullptr);
        CHECK(*results == L"42");

        results = itr.GetNextNumber<std::wstring>(
            DECIMALSEPARATORTYPE_NONE, SPECIAL_ITERATOR_ONNEWLINE_STOP);

        REQUIRE(results != nullptr);
        CHECK(*results == L"1");

        results = itr.GetNextCharacterSequence<std::wstring>(
            UNNORMALCHARACTER_TYPE_CONTROLCHARACTERS, SPECIAL_ITERATOR_ONNEWLINE_STOP);

        REQUIRE(results != nullptr);
        CHECK(*results == L"4 get until this");

        results = itr.GetUntilNextCharacterOrNothing<std::wstring>('[');

        CHECK(results != nullptr);

        results = itr.GetUntilNextCharacterOrNothing<std::wstring>(
            ';', SPECIAL_ITERATOR_ONNEWLINE_STOP);

        CHECK(results == nullptr);

        results = itr.GetStringInQuotes<std::wstring>(
            QUOTETYPE_BOTH, SPECIAL_ITERATOR_ONNEWLINE_STOP);

        REQUIRE(results != nullptr);
        CHECK(*results == L"how it cu");
    }

    SECTION("Getting until line end with escaped newline and windows style line change")
    {
        itr.ReInit(L"This is my line \\\r that has some things\r\n that are cut off");

        auto results = itr.GetUntilLineEnd<std::wstring>();

        REQUIRE(results != nullptr);
        CHECK(*results == L"This is my line \\\r that has some things");
    }

    SECTION("Getting until line end with proper style")
    {
        itr.ReInit(L"This is another line\nwith the right separator\nlast thing...");

        auto results = itr.GetUntilLineEnd<std::wstring>();

        REQUIRE(results != nullptr);
        CHECK(*results == L"This is another line");

        results = itr.GetUntilLineEnd<std::wstring>();

        REQUIRE(results != nullptr);
        CHECK(*results == L"with the right separator");

        results = itr.GetUntilLineEnd<std::wstring>();

        REQUIRE(results != nullptr);
        CHECK(*results == L"last thing...");
    }

    SECTION("Getting until string")
    {
        itr.ReInit(L"Get until ('bird') the word bird to remove junkwords");

        auto results = itr.GetUntilCharacterSequence<std::wstring>(L"bird");

        REQUIRE(results != nullptr);
        CHECK(*results == L"Get until ('bird') the word ");

        itr.MoveToNext();

        results = itr.GetUntilCharacterSequence<std::wstring>(L"word");

        REQUIRE(results != nullptr);
        CHECK(*results == L" to remove junk");
    }

    SECTION("Skipping whitespace with newlines")
    {
        itr.ReInit("{\n  \n}\n \n");

        CHECK(itr.GetCharacter() == '{');
        itr.MoveToNext();

        itr.SkipWhiteSpace(SPECIAL_ITERATOR_HANDLECOMMENTS_ASSTRING);

        CHECK(itr.GetCharacter() == '}');
    }

    SECTION("Command line parsing")
    {
        SECTION("Without quotes")
        {
            itr.ReInit("--cmd=Print(string(1)); --other-stuff");

            auto results = itr.GetNextCharacterSequence<std::string>(
                UNNORMALCHARACTER_TYPE_WHITESPACE | UNNORMALCHARACTER_TYPE_CONTROLCHARACTERS);

            REQUIRE(results != nullptr);
            CHECK(*results == "--cmd");

            if(itr.GetCharacter() == '=')
                itr.MoveToNext();

            results =
                itr.GetNextCharacterSequence<std::string>(UNNORMALCHARACTER_TYPE_WHITESPACE);

            REQUIRE(results);
            CHECK(*results == "Print(string(1));");
        }

        SECTION("With quotes")
        {
            itr.ReInit("--cmd='Print(string(1)); Print(string(2)); ' --other-stuff");

            auto results = itr.GetNextCharacterSequence<std::string>(
                UNNORMALCHARACTER_TYPE_WHITESPACE | UNNORMALCHARACTER_TYPE_CONTROLCHARACTERS);

            REQUIRE(results != nullptr);
            CHECK(*results == "--cmd");

            if(itr.GetCharacter() == '=')
                itr.MoveToNext();

            results =
                itr.GetNextCharacterSequence<std::string>(UNNORMALCHARACTER_TYPE_WHITESPACE);

            REQUIRE(results);
            CHECK(*results == "'Print(string(1)); Print(string(2)); '");

            StringIterator itr2(*results);

            CHECK(StringOperations::IsCharacterQuote(itr2.GetCharacter()));
            results = itr2.GetStringInQuotes<std::string>(QUOTETYPE_BOTH);

            REQUIRE(results);
            CHECK(*results == "Print(string(1)); Print(string(2)); ");
        }

        SECTION("Without equals")
        {
            itr.ReInit("--cmd Print(string(1)); --other-stuff");

            auto results = itr.GetNextCharacterSequence<std::string>(
                UNNORMALCHARACTER_TYPE_WHITESPACE | UNNORMALCHARACTER_TYPE_CONTROLCHARACTERS);

            REQUIRE(results != nullptr);
            CHECK(*results == "--cmd");

            if(itr.GetCharacter() == '=')
                itr.MoveToNext();

            results =
                itr.GetNextCharacterSequence<std::string>(UNNORMALCHARACTER_TYPE_WHITESPACE);

            REQUIRE(results);
            CHECK(*results == "Print(string(1));");
        }

        SECTION("whitespace")
        {
            SECTION("quotes") {}

            SECTION("No quotes")
            {
                itr.ReInit("--cmd= Print(string(1)); --other-stuff");

                auto results = itr.GetNextCharacterSequence<std::string>(
                    UNNORMALCHARACTER_TYPE_WHITESPACE |
                    UNNORMALCHARACTER_TYPE_CONTROLCHARACTERS);

                REQUIRE(results != nullptr);
                CHECK(*results == "--cmd");

                if(itr.GetCharacter() == '=')
                    itr.MoveToNext();

                results = itr.GetNextCharacterSequence<std::string>(
                    UNNORMALCHARACTER_TYPE_WHITESPACE);

                REQUIRE(results);
                CHECK(*results == "Print(string(1));");
            }
        }
    }
}

TEST_CASE("StringIterator UTF8 correctness", "[string][objectfile][utf8]")
{
    // Test UTF8 string handling //

    std::vector<int> unicodeholder = {
        0x00E4, '_', 0x0503, 0x04E8, 0x0A06, 0x1304, 0xAC93, 0x299D};

    std::string toutf8;

    utf8::utf32to8(unicodeholder.begin(), unicodeholder.end(), back_inserter(toutf8));

    std::wstring resultuni16;

    utf8::utf8to16(toutf8.begin(), toutf8.end(), back_inserter(resultuni16));


    StringIterator itr(std::make_unique<UTF8DataIterator>(toutf8));

    auto shouldbethesame = itr.GetUntilNextCharacterOrAll<std::string>('a');

    CHECK(*shouldbethesame == toutf8);

    itr.ReInit(std::make_unique<UTF8DataIterator>(
        "My Super nice \\= unicode is this : \"" + toutf8 + "\""));
    itr.GetUntilEqualityAssignment<std::string>(EQUALITYCHARACTER_TYPE_ALL);

    // Now get the UTF8 sequence //
    auto utf8encoded = itr.GetStringInQuotes<std::string>(QUOTETYPE_DOUBLEQUOTES);

    // Convert to utf 16 and compare //
    REQUIRE(utf8encoded != nullptr);

    std::wstring cvrtsy;

    utf8::utf8to16(utf8encoded->begin(), utf8encoded->end(), back_inserter(cvrtsy));

    CHECK(cvrtsy == resultuni16);
}

TEST_CASE("StringIterator bracket handling", "[string][objectfile]")
{
    StringIterator itr;
    std::unique_ptr<std::string> result;

    SECTION("Basic string")
    {
        itr.ReInit("my first [bracket value test] that is]] nice");
        result = itr.GetStringInBracketsRecursive<std::string>();

        REQUIRE(result);
        CHECK(*result == "bracket value test");

        itr.ReInit("[Another test]");

        result = itr.GetStringInBracketsRecursive<std::string>();

        REQUIRE(result);
        CHECK(*result == "Another test");
    }

    SECTION("Return NULL when missing end")
    {
        itr.ReInit("[Another test");

        result = itr.GetStringInBracketsRecursive<std::string>();

        REQUIRE_FALSE(result);
    }

    SECTION("Nesting all begin in a row")
    {
        itr.ReInit("[[[Nice deeply ] nested] brackets]");

        result = itr.GetStringInBracketsRecursive<std::string>();

        REQUIRE(result);
        CHECK(*result == "[[Nice deeply ] nested] brackets");
    }

    SECTION("Line end is ignored without flag")
    {
        itr.ReInit("[[[Nice deeply ]\n nested] brackets]");

        result = itr.GetStringInBracketsRecursive<std::string>();

        REQUIRE(result);
        CHECK(*result == "[[Nice deeply ]\n nested] brackets");
    }

    SECTION("Invalid end with line end when the flag specified")
    {
        itr.ReInit("[[[Nice deeply ]\n nested] brackets]");

        result =
            itr.GetStringInBracketsRecursive<std::string>(SPECIAL_ITERATOR_ONNEWLINE_STOP);

        REQUIRE_FALSE(result);
    }

    SECTION("Actual normal use data")
    {
        itr.ReInit("[[actual, usecases, [[things, other], final], last]]");

        result = itr.GetStringInBracketsRecursive<std::string>();

        REQUIRE(result);
        CHECK(*result == "[actual, usecases, [[things, other], final], last]");

        itr.ReInit(*result);

        result = itr.GetStringInBracketsRecursive<std::string>();

        REQUIRE(result);
        CHECK(*result == "actual, usecases, [[things, other], final], last");

        itr.ReInit(*result);

        result = itr.GetStringInBracketsRecursive<std::string>();

        REQUIRE(result);
        CHECK(*result == "[things, other], final");
    }
}


TEST_CASE("StringIterator new line handling", "[string]")
{
    StringIterator itr;
    std::unique_ptr<std::string> result;

    SECTION("Until windows multiline line separator")
    {
        itr.ReInit("Just a normal test string coming through");
        result =
            itr.GetUntilCharacterSequence<std::string>("str", SPECIAL_ITERATOR_ONNEWLINE_STOP);

        REQUIRE(result);
        CHECK(*result == "Just a normal test ");

        itr.ReInit("Just a normal \r\ntest string coming through");
        result =
            itr.GetUntilCharacterSequence<std::string>("str", SPECIAL_ITERATOR_ONNEWLINE_STOP);

        REQUIRE(result);
        CHECK(*result == "Just a normal ");

        CHECK(itr.GetCharacter() == 't');
        result = itr.GetNextCharacterSequence<std::string>(UNNORMALCHARACTER_TYPE_WHITESPACE);

        REQUIRE(result);
        CHECK(*result == "test");
    }

    SECTION("Until character new line before character")
    {
        itr.ReInit("true\n  another = thing\n   final = true;");

        result = itr.GetUntilNextCharacterOrAll<std::string>(
            ';', SPECIAL_ITERATOR_ONNEWLINE_STOP | SPECIAL_ITERATOR_HANDLECOMMENTS_ASSTRING);

        REQUIRE(result);
        CHECK(*result == "true");
    }
}
