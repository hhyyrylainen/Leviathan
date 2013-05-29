#ifndef LEVIATHAN_CONVERT
#define LEVIATHAN_CONVERT
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
namespace Leviathan{

	class Convert{
	public:
		//DLLEXPORT static string ToAscii( std::wstring& input );
		//DLLEXPORT static wstring ToUnicode( std::string& input );   
		/// string conversions

		DLLEXPORT static double DegreesToRadians(float degrees);
		DLLEXPORT static double RadiansToDegrees(float radians);


		DLLEXPORT static wstring IntToWstring(const int &i);
		DLLEXPORT static wstring FloatToWstring(const float &i);
		DLLEXPORT static wstring StringToWstring(const string &str);
		DLLEXPORT static wstring StringToWstringNonRef(const string str);
		DLLEXPORT static string WstringToString(const wstring &str);
		DLLEXPORT static wstring CharToWstring(const char &i);
		DLLEXPORT static int WstringToInt(const wstring &i);
		DLLEXPORT static int WstringFromBoolToInt(const wstring &i);
		DLLEXPORT static float WstringToFloat(const wstring &i);
		DLLEXPORT static wchar_t ToLower(const wchar_t &chara);


		// type checks //
		DLLEXPORT static int WstringTypeCheck(const wstring& data, int typecheckfor);
		DLLEXPORT static int WstringTypeNameCheck(const wstring &data);

		// template functions //

		template<typename T>
		DLLEXPORT static wstring ToWstring(const T& val){
			wstringstream stream;
			if(!(stream << val)){
				return L"";
			}
			return stream.str();
		}
		template<typename T>
		DLLEXPORT static string ToString(const T& val){
			stringstream stream;
			if(!(stream << val)){
				return "";
			}
			return stream.str();
		}

		template<class T>
		DLLEXPORT static wstring ToHexadecimalWstring(const T& val){
			wstringstream stream;
			if(!(stream << std::hex << val)){
				return L"";
			}
			return stream.str();
		}


	};

}

#endif