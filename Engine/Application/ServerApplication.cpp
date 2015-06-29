#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_SERVERAPPLICATION
#include "ServerApplication.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT Leviathan::ServerApplication::ServerApplication(){

}

DLLEXPORT Leviathan::ServerApplication::~ServerApplication(){

}
// ------------------------------------ //
DLLEXPORT bool Leviathan::ServerApplication::Initialize(AppDef* configuration){
	GUARD_LOCK();
	// store configuration //
	ApplicationConfiguration = configuration;

	// init engine //
	if(!_Engine->Init(ApplicationConfiguration, NETWORKED_TYPE_SERVER))
		return false;
	_InternalInit();
	return true;
}
// ------------------------------------ //




