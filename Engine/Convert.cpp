#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_CONVERT
#include "Convert.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
#include "DataBlock.h"

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
	//wstring stri;
	//stri.assign((str).begin(),(str).end());
	////stri.assign(str._my,(str).end());
	wstringstream strm;
	strm << str.c_str();
	return strm.str();
}
wstring Convert::StringToWstringNonRef(const string str){
	wstring stri;
	stri.assign(str.begin(),str.end());

	return stri;
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
int Convert::WstringToInt(const wstring &i){
	int ret=-1;
	wstringstream wstream;
	wstream.str(i.c_str());
	wstream >> ret;
	return ret;

}
int Convert::WstringFromBoolToInt(const wstring &i){
	if((i.compare(L"true") == 0) | (i.compare(L"True") == 0) | (i.compare(L"TRUE") == 0)){
		return true;	
	} else {
		return false;
	}
}

float Convert::WstringToFloat(const wstring &i){
	float ret=-1;
	wstringstream wstream;
	wstream.str(i.c_str());
	wstream >> ret;
	return ret;
}

wchar_t Convert::ToLower(const wchar_t &chara){
	int val = (int)chara;

	if((val <= 90) && (val >= 65)){
		return (wchar_t)(val+32);
	}
	return chara;
}
// ----------------- type checks ------------------- //
int Convert::WstringTypeCheck(const wstring& data, int typecheckfor){
	switch(typecheckfor){
	case 0: // int
		{
			wstring valid = L"1234567890-+";
			for(unsigned int i = 0; i < data.length(); i++){
				if(!Misc::WstringContains(valid, data[i]))
					return 0;
			}

			return 1;
		}
		break;
	case 1: // float/double
		{
			wstring valid = L"1234567890-+.,";
			for(unsigned int i = 0; i < data.length(); i++){
				if(!Misc::WstringContains(valid, data[i]))
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
				if(Misc::WstringContains(valid, data[i]))
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

int Convert::WstringTypeNameCheck(const wstring& data){
	if(Misc::WstringCompareInsensitive(data, L"int")){
		return DATABLOCK_TYPE_INT;
	}
	if(Misc::WstringCompareInsensitive(data, L"float")){
		return DATABLOCK_TYPE_FLOAT;
	}
	if(Misc::WstringCompareInsensitive(data, L"bool")){
		return DATABLOCK_TYPE_BOOL;
	}
	if(Misc::WstringCompareInsensitive(data, L"wstring")){
		return DATABLOCK_TYPE_WSTRING;
	}
	if(Misc::WstringCompareInsensitive(data, L"wstring")){
		return DATABLOCK_TYPE_WSTRING;
	}
	if(Misc::WstringCompareInsensitive(data, L"void*")){
		return DATABLOCK_TYPE_VOIDPTR;
	}
	return -1;
}