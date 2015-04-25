#pragma once
// ------------------------------------ //
#include "Define.h"
// ------------------------------------ //
#include <thread>


namespace Leviathan{
	class Misc{
	public:

        //! \brief Returns the directory from which the current process is ran
        //!
        //! As UTF-8 if possible
        DLLEXPORT static std::string GetProcessDirectory();

		template<typename T>
		DLLEXPORT static bool DoesVectorContainValue(const std::vector<T> &check, const T &val){
			for(size_t i = 0; i < check.size(); i++){
				if(*(check[i]) == *val)
					return true;
			}
			return false;
		}
		template<typename T>
		DLLEXPORT static bool DoesVectorContainValuePointers(const std::vector<T*> &check,
            const T* val)
        {
			for(unsigned int i = 0; i < check.size(); i++){
				if((check[i]) == val)
					return true;
			}
			return false;
		}

		// Operating system independent functions //
		DLLEXPORT static void KillThread(std::thread &threadtokill);

		DLLEXPORT static bool CompareDataBlockTypeToTHISNameCheck(int datablock,
            int typenamecheckresult);
	};
}


