#pragma once
// ------------------------------------ //
#include "Define.h"
// ------------------------------------ //
#include <atomic>
#include <limits.h>

namespace Leviathan{

	class IDFactory : ThreadSafe{
	public:
		DLLEXPORT IDFactory();
		DLLEXPORT ~IDFactory();


		DLLEXPORT static inline int GetID(){

			return Instance->ProduceID();
		}
        
		DLLEXPORT static inline int GetSystemID(){

			return Instance->ProduceSystemID();
		}

		DLLEXPORT int ProduceID(){

            const auto result = GlobalID.fetch_add(1, std::memory_order_relaxed);
            
            if(result == INT_MAX){
                
                Logger::Get()->Error("IDFactory ID overflow");
            }

            return result;
        }
        
		DLLEXPORT int ProduceSystemID(){

            const auto result = SystemID.fetch_add(1, std::memory_order_relaxed);
            
            if(result == INT_MAX){
                
                Logger::Get()->Error("IDFactory system ID overflow");
            }

            return result;
        }


		DLLEXPORT static IDFactory* Get();

	private:
        
        std::atomic_int SystemID;
        std::atomic_int GlobalID;

		static IDFactory* Instance;
	};

}

