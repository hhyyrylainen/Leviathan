#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_MASTERSERVERAPPLICATION
#include "MasterServerApplication.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT Leviathan::MasterServerApplication::MasterServerApplication(){

}

DLLEXPORT Leviathan::MasterServerApplication::~MasterServerApplication(){

}
// ------------------------------------ //
DLLEXPORT bool Leviathan::MasterServerApplication::Initialize(AppDef* configuration){
	// store configuration //
	ApplicationConfiguration = configuration;

	// init engine //
	if(!_Engine->Init(ApplicationConfiguration, NETWORKED_TYPE_MASTER))
		return false;
	_InternalInit();
	return true;
}
// ------------------------------------ //

// ------------------------------------ //

// ------------------------------------ //




