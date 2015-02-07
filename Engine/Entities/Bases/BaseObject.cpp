#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_BASEOBJECT
#include "BaseObject.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT Leviathan::BaseObject::BaseObject(int id, GameWorld* worldptr) : ID(id), OwnedByWorld(worldptr){

}

DLLEXPORT Leviathan::BaseObject::BaseObject() : ID(-1), OwnedByWorld(NULL){

}

BaseObject::~BaseObject(){

}
// ------------------------------------ //
DLLEXPORT void Leviathan::BaseObject::ReleaseData(){
	// default release, do nothing //
}
// ------------------------------------ //
void Leviathan::BaseObject::Disown(){
    GUARD_LOCK_THIS_OBJECT();

    OwnedByWorld = NULL;
}
// ------------------------------------ //

// ------------------------------------ //
