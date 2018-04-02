// ------------------------------------ //
#include "IDFactory.h"
using namespace Leviathan;
// ------------------------------------ //
IDFactory::IDFactory() : SystemID(1), GlobalID(1){
	Instance = this;
}
IDFactory::~IDFactory(){
	Instance = NULL;
}

DLLEXPORT IDFactory* Leviathan::IDFactory::Instance = NULL;
// ------------------------------------ //
DLLEXPORT IDFactory* Leviathan::IDFactory::Get(){

	return Instance;
}
// ------------------------------------ //
DLLEXPORT int IDFactory::ProduceID(){

    const auto result = GlobalID.fetch_add(1, std::memory_order_relaxed);
            
    if(result == INT_MAX){
        
        Logger::Get()->Error("IDFactory ID overflow");
    }

    return result;
}

DLLEXPORT int IDFactory::ProduceSystemID(){

    const auto result = SystemID.fetch_add(1, std::memory_order_relaxed);
            
    if(result == INT_MAX){
                
        Logger::Get()->Error("IDFactory system ID overflow");
    }

    return result;
}


