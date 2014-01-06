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
	ObjectLock guard(*this);
	// store configuration //
	ApplicationConfiguration = configuration;

	// init engine //
	if(!_Engine->Init(ApplicationConfiguration, NETWORKED_TYPE_SERVER))
		return false;
	_InternalInit();
	return true;
}

DLLEXPORT void Leviathan::ServerApplication::Release(){
	ObjectLock guard(*this);
	// set as quitting //
	Quit = true;

	// let engine release itself and then delete it //
	SAFE_RELEASEDEL(_Engine);
	// configuration object needs to be destroyed by the program main function //
}
// ------------------------------------ //




