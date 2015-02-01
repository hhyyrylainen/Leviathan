#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_CONVERT
#include "Convert.h"
#endif
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
#include "Common/DataStoring/DataBlock.h"
#include "Common/StringOperations.h"
#include "utf8/checked.h"

double Convert::DegreesToRadians(float degrees){
	return (degrees*(PI/180.f));
	
}
double Convert::RadiansToDegrees(float radians){
	return radians*(180.f/PI);
}


// ------------------------------------- //

wstring Convert::IntToWstring(const int &i){
	wstring replace;
	wstringstream wstream;
	wstream << i;
	replace=wstream.str();
	return replace;
}
wstring Convert::FloatToWstring(const float &i){
	wstring replace;
	wstringstream wstream;
	wstream << i;
	replace=wstream.str();
	return replace;
}
wstring Convert::StringToWstring(const string &str){
	wstringstream strm;
	strm << str.c_str();
	return strm.str();
}

DLLEXPORT std::wstring Leviathan::Convert::CharPtrToWstring(const char* charsource){
	wstringstream strm;
	strm << charsource;
	return strm.str();
}

string Convert::WstringToString(const wstring &str){
	string stri;
	stri.assign(str.begin(),str.end());

	return stri;
}
wstring Convert::CharToWstring(const char &i){
	wstring replace;
	wstringstream wstream;
	wstream << i;
	replace=wstream.str();
	return replace;
}
int Convert::WstringFromBoolToInt(const wstring &i){
	if((i.compare(L"true") == 0) || (i.compare(L"True") == 0) || (i.compare(L"TRUE") == 0) || (i.compare(L"1") == 0)){
		return true;	
	} else {
		return false;
	}
}
int Convert::StringFromBoolToInt(const string &i){
	if((i.compare("true") == 0) || (i.compare("True") == 0) || (i.compare("TRUE") == 0) || (i.compare("1") == 0)){
		return true;	
	} else {
		return false;
	}
}


DLLEXPORT bool Leviathan::Convert::IsWstringBool(const wstring &val, bool* valreceiver /*= NULL*/){
	wstring lowercased;
	Convert::ToLower(val, lowercased);

	if((lowercased.compare(L"true") == 0) || (lowercased.compare(L"1") == 0)){

		// set result if somebody wants it //
		if(valreceiver != NULL)
			*valreceiver = true;
		// was a boolean value //
		return true;
	} else if ((lowercased.compare(L"false") == 0) || (lowercased.compare(L"0") == 0)){

		// set result if somebody wants it //
		if(valreceiver != NULL)
			*valreceiver = false;
		// was a boolean value //
		return true;
	}
	// didn't match true/false //
	return false;
}

DLLEXPORT void Leviathan::Convert::ToLower(const wstring &source, wstring &target){
	// make sizes match //
	target.resize(source.size());

	for(size_t i = 0; i < source.size(); i++){
		if(source[i] >= L'A' && source[i] <= L'Z'){

			target[i] = 32+(int)source[i];
		} else {
			// just copy value //
			target[i] = source[i];
		}
	}
}

DLLEXPORT void Leviathan::Convert::ToCapital(const wstring &source, wstring &target){
	// make sizes match //
	target.resize(source.size());

	for(size_t i = 0; i < source.size(); i++){
		if(source[i] >= L'a' && source[i] <= L'z'){

			target[i] = ((int)source[i])-32;
		} else {
			// just copy value //
			target[i] = source[i];
		}
	}
}

wchar_t Convert::ToLower(const wchar_t &chara){
	int val = (int)chara;

	if((val <= 90) && (val >= 65)){
		return (wchar_t)(val+32);
	}
	return chara;
}
// ------------------------------------ //
DLLEXPORT std::wstring Leviathan::Convert::Utf8ToUtf16(const string &utf8str){

	// Store enough memory for the result //
	wstring results;
	results.reserve(utf8str.size());

	try {

		utf8::utf8to16(utf8str.begin(), utf8str.end(), back_inserter(results));

	} catch(utf8::invalid_code_point &e1){

		Logger::Get()->Error(L"Convert: invalid utf code point: "+Convert::ToWstring(e1.code_point()));
		return wstring();

	} catch(utf8::not_enough_room&){

		Logger::Get()->Error(L"Convert: not enough memory for string conversion");
		return wstring();
	}

	// The result is now done //
	return results;
}

DLLEXPORT std::string Leviathan::Convert::Utf16ToUtf8(const wstring &utf16str){
	// Store enough memory for the result //
	string results;
	results.reserve(utf16str.size());

	try {

		utf8::utf16to8(utf16str.begin(), utf16str.end(), back_inserter(results));

	} catch(utf8::invalid_code_point &e1){

		Logger::Get()->Error(L"Convert: invalid utf code point: "+Convert::ToWstring(e1.code_point()));
		return string();

	} catch(utf8::not_enough_room&){

		Logger::Get()->Error(L"Convert: not enough memory for string conversion");
		return string();
	}

	// The result is now done //
	return results;
}
// ----------------- type checks ------------------- //
int Convert::WstringTypeCheck(const wstring& data, int typecheckfor){
	switch(typecheckfor){
	case 0: // int
		{
			wstring valid = L"1234567890-+";
			for(unsigned int i = 0; i < data.length(); i++){
				if(!valid.find(data[i]))
					return 0;
			}

			return 1;
		}
		break;
	case 1: // float/double
		{
			wstring valid = L"1234567890-+.,";
			for(unsigned int i = 0; i < data.length(); i++){
				if(!valid.find(data[i]))
					return 0;
			}

			return 1;
		}
		break;
	case 3: // boolean
		{
			if((data.compare(L"true") != 0) && (data.compare(L"false") != 0) && (data.compare(L"True") != 0) && (data.compare(L"False") != 0) && (data.compare(L"TRUE") != 0) && (data.compare(L"FALSE") != 0)){
				// didn't match any //
				return 0;

			}
			return 1;

		}
		break;
	case 4: // wstring checking
		{
			unsigned int foundnumbparts = 0;
			wstring valid = L"1234567890-+.,";
			for(unsigned int i = 0; i < data.length(); i++){
				if(valid.find(data[i]))
					foundnumbparts++;
			}
			if(foundnumbparts == data.length())
				return 0;
			return 1;
		}
		break;


	}

	Logger::Get()->Error(L"WstringTypeCheck: invalid tocheck value", typecheckfor);
	return 007;
}

int Convert::WstringTypeNameCheck(const wstring &data){
	if(StringOperations::CompareInsensitive(data, wstring(L"int"))){
		return DATABLOCK_TYPE_INT;
	}
	if(StringOperations::CompareInsensitive(data, wstring(L"float"))){
		return DATABLOCK_TYPE_FLOAT;
	}
	if(StringOperations::CompareInsensitive(data, wstring(L"bool"))){
		return DATABLOCK_TYPE_BOOL;
	}
	if(StringOperations::CompareInsensitive(data, wstring(L"wstring"))){
		return DATABLOCK_TYPE_WSTRING;
	}
	if(StringOperations::CompareInsensitive(data, wstring(L"string"))){
		return DATABLOCK_TYPE_STRING;
	}
	if(StringOperations::CompareInsensitive(data, wstring(L"void*"))){
		return DATABLOCK_TYPE_VOIDPTR;
	}
	return -1;
}
// ------------------------------------ //
namespace Leviathan{
    
    template<> DLLEXPORT std::string Convert::ToString<Float4>(const Float4 &val){
            
        std::stringstream stream;
        if(!(stream << "[" << val.X << ", " << val.Y << ", " << val.Z << ", " << val.W <<
                "]"))
        {
            return "";
        }
            
        return stream.str();
    }

    template<> DLLEXPORT std::string Convert::ToString<Float3>(const Float3 &val){
            
        std::stringstream stream;
        if(!(stream << "[" << val.X << ", " << val.Y << ", " << val.Z << ", " << "]"))
        {
            return "";
        }
            
        return stream.str();
    }

}
