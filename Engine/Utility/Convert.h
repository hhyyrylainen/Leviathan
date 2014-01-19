#ifndef LEVIATHAN_CONVERT
#define LEVIATHAN_CONVERT
// ------------------------------------ //
// Reduce bloat in precompiled header
// ------------------------------------ //
// ---- includes ---- //
namespace Leviathan{

#define STRINGTOSOMETHINGTEMPLATEALTERNATIVE(StringyType, strstreamt, funcname, totype) DLLEXPORT static inline totype funcname(const StringyType &str){totype tempval; strstreamt stream;stream.str(str.c_str());stream >> tempval;return tempval;}
	
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

		template<class StringStreamType, class ReturnType>
		DLLEXPORT static ReturnType Float3ToSWstring(const Float3 &data){

			StringStreamType stream;

			stream << "[" << data.X << ", " << data.Y << ", " << data.Z << "]";
			return stream.str();
		}

		template<class StringStreamType, class ReturnType>
		DLLEXPORT static ReturnType Float4ToString(const Float4 &data){

			StringStreamType stream;

			stream << "[" << data.X << ", " << data.Y << ", " << data.Z << ", " << data.W << "]";
			return stream.str();
		}

		DLLEXPORT static int WstringFromBoolToInt(const wstring &i);
		DLLEXPORT static int StringFromBoolToInt(const string &i);

		DLLEXPORT static bool IsWstringBool(const wstring &val, bool* valreceiver = NULL);

		template<class T>
		DLLEXPORT static inline T WstringTo(const wstring &str){
			T tempval;
			wstringstream stream;
			stream.str(str.c_str());
			stream >> tempval;
			return tempval;
		}
		template<class T>
		DLLEXPORT static inline T StringTo(const string &str){
			T tempval;
			stringstream stream;
			stream.str(str.c_str());
			stream >> tempval;
			return tempval;
		}


		// macro conversions //
		STRINGTOSOMETHINGTEMPLATEALTERNATIVE(wstring, wstringstream, WstringToInt, int);
		STRINGTOSOMETHINGTEMPLATEALTERNATIVE(wstring, wstringstream, WstringToFloat, float);


		DLLEXPORT static wchar_t ToLower(const wchar_t &chara);
		DLLEXPORT static void ToLower(const wstring &source, wstring &target);
		DLLEXPORT static void ToCapital(const wstring &source, wstring &target);

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