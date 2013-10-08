#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_IDFACTORY
#include "IDFactory.h"
#endif
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
DLLEXPORT int Leviathan::IDFactory::ProduceID(boost::strict_lock<IDFactory> &guard){
	VerifyLock(guard);
	// we are safely locked and can perform the action //
	GlobalID++;
	if(GlobalID == INT_MAX){
		Logger::Get()->Error(L"IDFactory GlobalID overflow", INT_MAX);
	}

	return GlobalID;
}

DLLEXPORT int Leviathan::IDFactory::ProduceSystemID(boost::strict_lock<IDFactory> &guard){
	VerifyLock(guard);
	// we are safely locked and can perform the action //
	SystemID++;
	return SystemID;
}

DLLEXPORT IDFactory* Leviathan::IDFactory::Get(){
	// create instance if doesn't exist //
	if(!Instance){
		Instance = new IDFactory();
	}
	return Instance;
}


// ------------------------------------ //