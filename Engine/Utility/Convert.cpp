// ------------------------------------ //
#include "Convert.h"

#include "Define.h"
#include "Common/DataStoring/DataBlock.h"
#include "../Common/StringOperations.h"
#include "utf8/checked.h"
using namespace Leviathan;
using namespace std;
// ------------------------------------ //
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
    if(StringOperations::CompareInsensitive<wstring>(i, L"true") || i == L"1"){
		return true;	
	} else {
		return false;
	}
}
int Convert::StringFromBoolToInt(const string &i){
    if(StringOperations::CompareInsensitive<string>(i, "true") || i == "1"){
		return true;	
	} else {
		return false;
	}
}

DLLEXPORT bool Convert::IsStringBool(const string &val, bool* receiver){

    if(StringOperations::CompareInsensitive<string>(val, "true") || val == "1"){

        *receiver = true;
        return true;
    }

    if(StringOperations::CompareInsensitive<string>(val, "false") || val == "0"){

        *receiver = false;
        return true;
    }

	return false;
}

DLLEXPORT int Convert::StringTypeNameCheck(const std::string &name){

    if(name == "int")
        return DATABLOCK_TYPE_INT;
    if(name == "float")
        return DATABLOCK_TYPE_FLOAT;
    if(name == "bool")
        return DATABLOCK_TYPE_BOOL;
    if(name == "wstring")
        return DATABLOCK_TYPE_WSTRING;
    if(name == "string")
        return DATABLOCK_TYPE_STRING;
    if(name == "char")
        return DATABLOCK_TYPE_CHAR;
    if(name == "double")
        return DATABLOCK_TYPE_DOUBLE;
    if(name == "void" || name == "void*")
        return DATABLOCK_TYPE_VOIDPTR;

    return -1;
}
// ------------------------------------ //
DLLEXPORT std::wstring Leviathan::Convert::Utf8ToUtf16(const string &utf8str){

	// Store enough memory for the result //
	wstring results;
	results.reserve(utf8str.size());

	try {

		utf8::utf8to16(utf8str.begin(), utf8str.end(), back_inserter(results));

	} catch(utf8::invalid_code_point &e1){

		Logger::Get()->Error("Convert: invalid utf code point: "+
            Convert::ToString(e1.code_point()));
        
		return wstring();

	} catch(utf8::not_enough_room&){

		Logger::Get()->Error("Convert: not enough memory for string conversion");
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

		Logger::Get()->Error("Convert: invalid utf code point: "+
            Convert::ToString(e1.code_point()));
		return string();

	} catch(utf8::not_enough_room&){

		Logger::Get()->Error("Convert: not enough memory for string conversion");
		return string();
	}

	// The result is now done //
	return results;
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
