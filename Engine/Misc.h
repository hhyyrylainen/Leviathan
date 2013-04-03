#ifndef LEVIATHAN_MISC
#define LEVIATHAN_MISC
// ------------------------------------ //
#ifndef LEVIATHAN_INCLUDE
#include "Include.h"
#endif
// ------------------------------------ //
// ---- includes ---- //


namespace Leviathan{
	class Misc{
	public:

		///time functions
		DLLEXPORT static __int64 GetTimeMs64();
		DLLEXPORT static __int64 GetTimeMicro64();
		///reduce code
		template<typename T>
		DLLEXPORT static bool IsNumber(T& Value){
			return (Value == Value);
		}
		template<typename T>
		DLLEXPORT static bool IsFiniteNumber(T& Value){
			// fpclass check //
			switch (_fpclass(Value))
			{
			case _FPCLASS_SNAN: return false;
			case _FPCLASS_QNAN: return false;
			case _FPCLASS_NINF: return false;
			//case _FPCLASS_NN: return false;
			//case _FPCLASS_ND: return false;
			//case _FPCLASS_NZ: return false;
			//case _FPCLASS_PZ: return false;
			//case _FPCLASS_PD: return false;
			//case _FPCLASS_PN: return false;
			case _FPCLASS_PINF: return false;
			}

			return (true);
		}
		DLLEXPORT static inline bool ToggleBool(bool &Value){
			if(Value){
				Value = false;
				return false;
			}
			Value = true;
			return true;
		}

		template<typename T>
		DLLEXPORT static bool DoesVectorContainValue(const vector<T> &check, const T& val){
			for(unsigned int i = 0; i < check.size(); i++){
				if(*(check[i]) == *val)
					return true;
			}
			return false;
		}
		template<typename T>
		DLLEXPORT static bool DoesVectorContainValuePointers(const vector<T*> &check, const T* val){
			for(unsigned int i = 0; i < check.size(); i++){
				if((check[i]) == val)
					return true;
			}
			return false;
		}
		template<typename T>
		DLLEXPORT static int ReplaceValuesInVector(vector<T> &check, const T& lookfor, const T& replacewith){
			int Count = 0;
			for(unsigned int i = 0; i < check.size(); i++){
				if((check[i]) == lookfor){
					check[i] = replacewith;
					Count++;
				}
			}
			return Count;
		}
		template<typename T>
		DLLEXPORT static T VectorValuesToSingle(vector<T*> &values, const T& separator, bool UseSeparator = false){
			//bool IsFirst = true;
			if(values.size() == 0)
				return T();
			T temp = *values[0];

			for(unsigned int i = 1; i < values.size(); i++){
				if(UseSeparator)
					temp += separator;
				temp += *values[i];
			}

			return temp;
		}
		template<typename T>
		DLLEXPORT static T VectorValuesToSingle(vector<T> &values, const T& separator, bool UseSeparator = false){
			//bool IsFirst = true;
			if(values.size() == 0)
				return T();
			T temp = values[0];

			for(unsigned int i = 1; i < values.size(); i++){
				if(UseSeparator)
					temp += separator;
				temp += values[i];
			}

			return temp;
		}

		///string operations
		DLLEXPORT static int CutWstring(const wstring& strtocut,const wstring &separator, vector<wstring>& vec);
		DLLEXPORT static int CountOccuranceWstring(const wstring& data,wstring lookfor);
		DLLEXPORT static wstring Replace(const wstring& data, const wstring &toreplace, const wstring &replacer);
		DLLEXPORT static void ReplaceWord(wstring& data, wstring toreplace, wstring replacer);
		DLLEXPORT static bool WstringContains(const wstring& data, wchar_t check);
		DLLEXPORT static bool WstringContainsNumbers(const wstring& data);
		DLLEXPORT static int WstringGetSecondWord(const wstring& data, wstring& result);
		DLLEXPORT static int WstringGetFirstWord(const wstring& data, wstring& result);
		DLLEXPORT static bool WstringStartsWith(const wstring& data, wstring lookfor);
		DLLEXPORT static wstring WstringRemoveFirstWords(wstring& data, int amount);
		DLLEXPORT static wstring WstringStitchTogether(vector<wstring*> data, wstring separator);
		// returns 0 for equal 1 for str is before and -1 for tocompare to be before //
		DLLEXPORT static int IsWstringBeforeInAlphabet(const wstring& str, const wstring& tocompare);
			// type checks //
		DLLEXPORT static int WstringTypeCheck(const wstring& data, int typecheckfor);
		DLLEXPORT static int WstringTypeNameCheck(const wstring &data);
		DLLEXPORT static bool CompareDataBlockTypeToTHISNameCheck(int datablock, int typenamecheckresult);
		DLLEXPORT static bool WstringCompareInsensitive(const wstring& data, wstring second);
		DLLEXPORT static bool WstringCompareInsensitiveRefs(const wstring& data, const wstring &second);
		DLLEXPORT static bool IsCharacterNumber(wchar_t chara);

		

		DLLEXPORT static wstring& GetErrString();
		DLLEXPORT static wstring GetValidCharacters();

		static wstring EmptyString;
		static wstring ValidNumberCharacters;

	private:
		static wstring Errstring;


	};
	

	/// quickmessage


}

#endif