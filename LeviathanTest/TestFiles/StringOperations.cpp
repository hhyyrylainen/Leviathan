
/**
   \file Testing for methods in StringOperations
*/
#include "Common/StringOperations.h"

#include "catch.hpp"

using namespace Leviathan;
using namespace std;

// First test duplicated for wstring and string others are string only as wstring versions
// *should* also work if they pass
TEST_CASE("Wstring cutting", "[string]"){

	wstring teststring = L"hey nice string -ql-you have -qlhere or -ql-mql-yaybe-ql- oh no! "
        "-qltest -ql-over or not-ql?";
    
	vector<wstring> exampleresult;
	exampleresult.reserve(5);
	exampleresult.push_back(L"hey nice string ");
	exampleresult.push_back(L"you have -qlhere or ");
	exampleresult.push_back(L"mql-yaybe");
	exampleresult.push_back(L" oh no! -qltest ");
	exampleresult.push_back(L"over or not-ql?");

	vector<wstring> result;

	StringOperations::CutString(teststring, wstring(L"-ql-"), result);

    REQUIRE(result.size() == exampleresult.size());
    
    for(size_t i = 0; i < exampleresult.size(); i++){

        REQUIRE(result[i] == exampleresult[i]);
    }
}

TEST_CASE("String cutting", "[string]"){

	string teststring = "hey nice string -ql-you have -qlhere or -ql-mql-yaybe-ql- oh no! "
        "-qltest -ql-over or not-ql?";
    
	vector<string> exampleresult;
	exampleresult.reserve(5);
	exampleresult.push_back("hey nice string ");
	exampleresult.push_back("you have -qlhere or ");
	exampleresult.push_back("mql-yaybe");
	exampleresult.push_back(" oh no! -qltest ");
	exampleresult.push_back("over or not-ql?");

	vector<string> result;

	StringOperations::CutString(teststring, string("-ql-"), result);

    REQUIRE(result.size() == exampleresult.size());
    
    for(size_t i = 0; i < exampleresult.size(); i++){

        REQUIRE(result[i] == exampleresult[i]);
    }
}

TEST_CASE("String text replacing ", "[string]"){

	const string teststring = "hey nice string -ql-you have -qlhere or -ql-mql-yaybe-ql- oh no! -qltest -ql-over "
        "or not-ql?";
	const string correctresult = "hey nice string hey_whatsthis?you have -qlhere or hey_whatsthis?mql-yaybehey_"
        "whatsthis? oh no! -qltest hey_whatsthis?over or not-ql?";

	string result = StringOperations::Replace<string>(teststring, "-ql-", "hey_whatsthis?");

    REQUIRE(result == correctresult);
}


TEST_CASE("String whitespace trimming", "[string]"){

    const string testdata = "		a  asd	hey nice   ?!	 ";

	string result = testdata;
	const string correctresult = "a  asd	hey nice   ?!";

	StringOperations::RemovePreceedingTrailingSpaces(result);

    REQUIRE(result == correctresult);
}

#define TESTPATHPLAIN	"My/super/path/filesthis.extsuper"

#define TESTPATHSTR			"My/super/path/filesthis.extsuper"
#define TESTPATHSTRWIN		"My\\super\\path\\filesthis.extsuper"


TEST_CASE("StringOperations common work with string and wstring", "[string]"){

	wstring paththing = Convert::StringToWstring(TESTPATHPLAIN);
	string paththing2 = TESTPATHSTRWIN;


	wstring result = StringOperations::RemoveExtension<wstring, wchar_t>(paththing, true);
	string result2 = StringOperations::RemoveExtension<string, char>(paththing2, true);

	wstring wstringified = Convert::StringToWstring(result2);

    CHECK(result == wstringified);
    CHECK(result == L"filesthis");

	result = StringOperations::GetExtension<wstring, wchar_t>(paththing);

	CHECK(result == L"extsuper");

	result = StringOperations::ChangeExtension<wstring, wchar_t>(paththing, L"superier");

	CHECK(result == L"My/super/path/filesthis.superier");

	wstring ressecond = StringOperations::RemovePath<wstring, wchar_t>(result);

	CHECK(ressecond == L"filesthis.superier");

	string removed = StringOperations::RemovePathString("./GUI/Nice/Panels/Mytexture.png");

	CHECK(removed == "Mytexture.png");

	wstring pathy = StringOperations::GetPathWstring(paththing);

	CHECK(pathy == L"My/super/path/");

	CHECK(StringOperations::StringStartsWith(wstring(L"My super text"), wstring(L"My")));
    CHECK(StringOperations::StringStartsWith(wstring(L"}"), wstring(L"}")));
    CHECK_FALSE(StringOperations::StringStartsWith(string("This shouldn't match"),
            string("this")));

	// Line end changing //
	wstring pathtestoriginal = L"My text is quite nice\nand has\n multiple\r\n lines\n"
        L"that are separated\n";

	wstring pathresult = StringOperations::ChangeLineEndsToWindowsWstring(pathtestoriginal);

	CHECK(pathresult == L"My text is quite nice\r\nand has\r\n multiple\r\n lines\r\nthat are separated\r\n");

	wstring backlinetest = StringOperations::ChangeLineEndsToUniversalWstring(pathresult);

	CHECK(backlinetest == L"My text is quite nice\nand has\n multiple\n lines\nthat are separated\n");
}


