#ifndef LEVIATHAN_MISC
#define LEVIATHAN_MISC
// ------------------------------------ //
#ifndef LEVIATHAN_INCLUDE
#include "Include.h"
#endif
#include <boost/thread.hpp>
// ------------------------------------ //
// ---- includes ---- //


namespace Leviathan{
	class Misc{
	public:

		///time functions
		DLLEXPORT static __int64 GetTimeMs64();
		DLLEXPORT static __int64 GetTimeMicro64();
		//! \brief Gets the current time in a thread safe way
		//!
		//! This should always be used when getting the time to avoid segfaulting
		//! \note boost::chrono::high_resolution_clock should be the exact same thing as WantedClockType
		//! it is used because Define.h can't be included in this file
		DLLEXPORT static boost::chrono::high_resolution_clock::time_point GetThreadSafeSteadyTimePoint();

        //! \brief Returns the directory from which the current process is ran
        //!
        //! As UTF-8 if possible
        DLLEXPORT static std::string GetProcessDirectory();

		///reduce code
		template<typename T>
		DLLEXPORT static bool IsNumber(T& Value){
			return (Value == Value);
		}
		template<typename T>
		DLLEXPORT static bool IsFiniteNumber(T& Value){
#ifdef _WIN32
			// fp class check //
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
#else
			return isfinite(Value);
#endif
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
		DLLEXPORT static bool DoesVectorContainValue(const vector<T> &check, const T &val){
			for(size_t i = 0; i < check.size(); i++){
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
		DLLEXPORT static T VectorValuesToSingleSmartPTR(vector<shared_ptr<T>> &values, const T& separator, bool UseSeparator = false){
			if(values.size() == 0)
				return T();
			T temp(*(values[0].get()));

			for(unsigned int i = 1; i < values.size(); i++){
				if(UseSeparator)
					temp += separator;
				temp += *values[i].get();
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

		// Operating system independent functions //
		DLLEXPORT static void KillThread(boost::thread &threadtokill);



		DLLEXPORT static bool CompareDataBlockTypeToTHISNameCheck(int datablock, int typenamecheckresult);

		DLLEXPORT static inline wstring& GetErrString(){

			return Errstring;
		}
		DLLEXPORT static inline const wstring& GetErrStringConst(){

			return Errstring;
		}
		DLLEXPORT static inline string& GetErrStrings(){

			return Errstrings;
		}
		DLLEXPORT static inline const string& GetErrStringConsts(){

			return Errstrings;
		}

		static wstring EmptyString;
		static wstring ValidNumberCharacters;

	private:
		static wstring Errstring;
		static string Errstrings;


	};
}

#endif
