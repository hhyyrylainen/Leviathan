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


	// Stress testing //
	for(int i = 0; i < tests; i++){

		wstring resthing = RemoveExtension<wstring, wchar_t>(paththing, true);

	}


	// Didn't fail //
	return false;
}
// ------------------------------------ //

// ------------------ StringConstants definitions ------------------ //
template<> const wchar_t WstringConstants::DotCharacter = L'.';
template<> const wchar_t WstringConstants::UniversalPathSeparator = L'/';
template<> const wchar_t WstringConstants::WindowsPathSeparator = L'\\';
template<> const wchar_t WstringConstants::SpaceCharacter = L' ';

template<> const wchar_t WstringConstants::FirstNumber = L'0';
template<> const wchar_t WstringConstants::LastNumber = L'9';
template<> const wchar_t WstringConstants::Dash = L'-';
template<> const wchar_t WstringConstants::PlusSymbol = L'+';
// ------------------------------------ //

// ------------------------------------ //

// ------------------------------------ //