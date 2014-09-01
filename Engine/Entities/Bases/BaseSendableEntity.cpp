#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_BASESENDABLEENTITY
#include "BaseSendableEntity.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT BaseSendableEntity::BaseSendableEntity(BaseEntitySerializer::TypeIDSize type) : SerializeType(type){


}

DLLEXPORT BaseSendableEntity::~BaseSendableEntity(){

}
// ------------------------------------ //

