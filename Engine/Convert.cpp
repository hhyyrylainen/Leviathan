#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_CONVERT
#include "Convert.h"
#endif
using namespace Leviathan;
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

// must be in class declaration //
//template<typename T>
//wstring Convert::ToWstring(const T& val){
//	wstringstream stream;
//	if(!(stream << val)){
//		return L"";
//	}
//	return stream.str();
//}
wchar_t Convert::ToLower(const wchar_t &chara){
	int val = (int)chara;

	if((val <= 90) && (val >= 65)){
		return (wchar_t)(val+32);
	}
	return chara;
}