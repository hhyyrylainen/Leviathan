#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_STRINGOPERATIONS
#include "StringOperations.h"
#endif
using namespace Leviathan;
// ------------------------------------ //

#define TESTPATHPLAIN	My/super/path/filesthis.extsuper

#define TESTPATHSTR			"My/super/path/filesthis.extsuper"
#define TESTPATHSTRWIN		"My\\super\\path\\filesthis.extsuper"

DLLEXPORT bool Leviathan::StringOperations::PerformTesting(const int &tests){
	// All tests should work with both wstring and string //

	wstring paththing = WSTRINGIFY(TESTPATHPLAIN);
	string paththing2 = TESTPATHSTRWIN;


	wstring result = RemoveExtension<wstring, wchar_t>(paththing, true);
	string result2 = RemoveExtension<string, char>(paththing2, true);

	wstring wstringified = Convert::StringToWstring(result2);

	if(result != wstringified || result != L"filesthis"){

		return true;
	}

	result = GetExtension<wstring, wchar_t>(paththing);

	if(result != L"extsuper"){

		return true;
	}

	result = ChangeExtension<wstring, wchar_t>(paththing, L"superier");

	if(result != L"My/super/path/filesthis.superier"){

		return true;
	}

	wstring ressecond = RemovePath<wstring, wchar_t>(result);

	if(ressecond != L"filesthis.superier"){

		return true;
	}

	string removed = RemovePathString("./GUI/Nice/Panels/Mytexture.png");

	if(removed != "Mytexture.png"){

		return true;
	}

	wstring pathy = GetPathWstring(paththing);

	if(pathy != L"My/super/path/"){

		return true;
	}


	if(!StringStartsWith(wstring(L"My super text"), wstring(L"My")) || !StringStartsWith(wstring(L"}"), wstring(L"}")) ||
		StringStartsWith(string("This shouldn't match"), string("this")))
	{

		return true;
	}

	// Line end changing //
	wstring pathtestoriginal = L"My text is quite nice\nand has\n multiple\r\n lines\n that are separated\n";

	wstring pathresult = ChangeLineEndsToWindowsWstring(pathtestoriginal);

	if(pathresult != L"My text is quite nice\r\nand has\r\n multiple\r\n lines\r\n that are separated\r\n"){

		return true;
	}

	wstring backlinetest = ChangeLineEndsToUniversalWstring(pathresult);

	if(backlinetest != L"My text is quite nice\nand has\n multiple\n lines\n that are separated\n"){

		return true;
	}


	// Stress testing //
	for(int i = 0; i < tests; i++){

		wstring resthing = RemoveExtension<wstring, wchar_t>(paththing, true);

	}


	// Didn't fail //
	return false;
}
// ------------------------------------ //


template<> const wstring Leviathan::StringConstants<wstring, wchar_t>::WindowsLineSeparator = L"\r\n";
template<> const wstring Leviathan::StringConstants<wstring, wchar_t>::UniversalLineSeparator = L"\n";


template<> const string Leviathan::StringConstants<string, char>::WindowsLineSeparator = "\r\n";
template<> const string Leviathan::StringConstants<string, char>::UniversalLineSeparator = "\n";