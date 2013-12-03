#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_BASEOBJECT
#include "BaseObject.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT Leviathan::BaseObject::BaseObject(int id, GameWorld* worldptr) : ID(id), OwnedByWorld(worldptr){

}

DLLEXPORT void Leviathan::BaseObject::ReleaseData(){
	// default release, do nothing //
}

BaseObject::~BaseObject(){

}
// ------------------------------------ //

// ------------------------------------ //

// ------------------------------------ //

// ------------------------------------ //