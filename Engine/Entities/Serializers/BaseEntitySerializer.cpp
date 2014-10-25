#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_BASEENTITYSERIALIZER
#include "BaseEntitySerializer.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT Leviathan::BaseEntitySerializer::BaseEntitySerializer(ENTITYSERIALIZEDTYPE type) : Type(type){

}

DLLEXPORT Leviathan::BaseEntitySerializer:: ~BaseEntitySerializer(){

}
// ------------------------------------ //
DLLEXPORT bool Leviathan::BaseEntitySerializer::CanSerializeType(TypeIDSize typetocheck) const{

    return Type == typetocheck;
}
// ------------------------------------ //

