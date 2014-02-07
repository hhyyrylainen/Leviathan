#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_BASENOTIFIABLEENTITY
#include "BaseNotifiableEntity.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT Leviathan::BaseNotifiableEntity::BaseNotifiableEntity() : BaseNotifiable(this){

}

DLLEXPORT Leviathan::BaseNotifiableEntity::~BaseNotifiableEntity(){

}
// ------------------------------------ //


