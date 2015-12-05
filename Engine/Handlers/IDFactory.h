#pragma once
// ------------------------------------ //
#include "Include.h"
#include <atomic>
#include <limits.h>
#include "../Logger.h"
#include "../Common/ThreadSafe.h"

namespace Leviathan{

	class IDFactory : public ThreadSafe{
	public:
		DLLEXPORT IDFactory();
		DLLEXPORT ~IDFactory();


		DLLEXPORT static inline int GetID(){

			return Instance->ProduceID();
		}
        
		DLLEXPORT static inline int GetSystemID(){

			return Instance->ProduceSystemID();
		}

		DLLEXPORT int ProduceID();
        
		DLLEXPORT int ProduceSystemID();

		DLLEXPORT static IDFactory* Get();

	private:
        
        std::atomic_int SystemID;
        std::atomic_int GlobalID;

		static IDFactory* Instance;
	};

}

