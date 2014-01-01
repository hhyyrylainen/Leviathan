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
	_Engine = new Engine(this);
	if(!_Engine->Init(ApplicationConfiguration, NETWORKED_TYPE_MASTER))
		return false;
	_InternalInit();
	return true;
}

DLLEXPORT void Leviathan::MasterServerApplication::Release(){
	// set as quitting //
	Quit = true;

	// let engine release itself and then delete it //
	SAFE_RELEASEDEL(_Engine);
	// configuration object needs to be destroyed by the program main function //
}
// ------------------------------------ //

// ------------------------------------ //

// ------------------------------------ //




