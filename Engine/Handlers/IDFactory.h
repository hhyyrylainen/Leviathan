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


		static inline int GetID(){

			return Instance->ProduceID();
		}
        
		static inline int GetSystemID(){

			return Instance->ProduceSystemID();
		}

		DLLEXPORT int ProduceID();
        
		DLLEXPORT int ProduceSystemID();

		DLLEXPORT static IDFactory* Get();

	private:
        
        std::atomic_int SystemID;
        std::atomic_int GlobalID;

		DLLEXPORT static IDFactory* Instance;
	};

}

