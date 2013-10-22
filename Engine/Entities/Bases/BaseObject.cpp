#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_BASEOBJECT
#include "BaseObject.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT Leviathan::BaseObject::BaseObject(int id) : ID(id){

}

DLLEXPORT void Leviathan::BaseObject::Release(){
	// default release, do nothing //
}

BaseObject::~BaseObject(){

}
// ------------------------------------ //

// ------------------------------------ //

// ------------------------------------ //

// ------------------------------------ //