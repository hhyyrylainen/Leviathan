// ------------------------------------ //
#ifndef LEVIATHAN_SENDABLEENTITYSERIALIZER
#include "SendableEntitySerializer.h"
#endif
using namespace Leviathan;
#include "Entities/Bases/BaseSendableEntity.h"
// ------------------------------------ //
DLLEXPORT SendableEntitySerializer::SendableEntitySerializer() :
    BaseEntitySerializer(ENTITYSERIALIZEDTYPE_SENDABLE_ENTITY)
{

    
}

DLLEXPORT SendableEntitySerializer::~SendableEntitySerializer(){


}
// ------------------------------------ //
DLLEXPORT bool SendableEntitySerializer::CreatePacketForConnection(BaseObject* object, sf::Packet &packet,
    ConnectionInfo* connectionptr)
{
    // Cast to the base class that can handle the required functions //
    BaseSendableEntity* asbase = dynamic_cast<BaseSendableEntity*>(object);

    if(!asbase)
        return false;

    // The type is correct, so we should always return true after this //
    
    // First add the type variable and then let the entity add it's data //
    packet << Type;

    asbase->SerializeToPacket(packet);

    return true;
}
// ------------------------------------ //
DLLEXPORT bool SendableEntitySerializer::DeserializeWholeEntityFromPacket(BaseObject** returnobj,
    int32_t serializetype, sf::Packet &packet, GameWorld* world)
{
    // Verify that the type matches //
    if(serializetype != Type){

        Logger::Get()->Error("SendableEntitySerializer: passed wrong serializetype to unserialize");
        return false;
    }

    // Create a sendable entity //
    auto sendableobj = BaseSendableEntity::UnSerializeFromPacket(packet, world);

    // Set the object if it was created //
    if(!sendableobj){

        (*returnobj) = NULL;
        
    } else {

        (*returnobj) = sendableobj.release();
    }


    return true;
}
// ------------------------------------ //
DLLEXPORT bool SendableEntitySerializer::ApplyUpdateFromPacket(BaseObject* targetobject, sf::Packet &packet){

    BaseSendableEntity* asbase = dynamic_cast<BaseSendableEntity*>(targetobject);

    if(!asbase)
        return false;

    DEBUG_BREAK;
}
// ------------------------------------ //
DLLEXPORT bool SendableEntitySerializer::IsObjectTypeCorrect(BaseObject* object) const{

    return dynamic_cast<BaseSendableEntity*>(object) ? true: false;
}

