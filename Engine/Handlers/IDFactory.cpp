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

IDFactory* Leviathan::IDFactory::Instance = NULL;
// ------------------------------------ //
DLLEXPORT IDFactory* Leviathan::IDFactory::Get(){

	return Instance;
}
// ------------------------------------ //

