// ------------------------------------ //
#include "SendableEntitySerializer.h"

#include "../../Utility/Convert.h"
#include "Entities/Bases/BaseSendableEntity.h"
using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT SendableEntitySerializer::SendableEntitySerializer() :
    BaseEntitySerializer(ENTITYSERIALIZEDTYPE_SENDABLE_ENTITY)
{

    
}

DLLEXPORT SendableEntitySerializer::~SendableEntitySerializer(){


}
// ------------------------------------ //
DLLEXPORT bool SendableEntitySerializer::CreatePacketForConnection(BaseObject* object,
    Lock &objectlock, sf::Packet &packet, ConnectionInfo* connectionptr)
{
    // Cast to the base class that can handle the required functions //
    BaseSendableEntity* asbase = dynamic_cast<BaseSendableEntity*>(object);

    if(!asbase)
        return false;

    // The type is correct, so we should always return true after this //
    
    // First add the type variable and then let the entity add it's data //
    packet << Type;

    // Add the connection to the known ones (this is before the serialize to make sure that all updates after
    // serializing get sent)
    asbase->AddConnectionToReceivers(objectlock, connectionptr);
    
    asbase->SerializeToPacket(objectlock, packet);

    return true;
}
// ------------------------------------ //
DLLEXPORT bool SendableEntitySerializer::DeserializeWholeEntityFromPacket(BaseObject** returnobj,
    int32_t serializetype, sf::Packet &packet, int objectid, GameWorld* world)
{
    // Verify that the type matches //
    if(serializetype != Type){

        Logger::Get()->Error("SendableEntitySerializer: passed wrong serializetype to unserialize, Our: "+
            Convert::ToString(Type)+", got: "+Convert::ToString(serializetype));
        return false;
    }

    // Create a sendable entity //
    auto sendableobj = BaseSendableEntity::UnSerializeFromPacket(packet, world, objectid);

    // Set the object if it was created //
    if(!sendableobj){

        (*returnobj) = NULL;
        
    } else {

        (*returnobj) = sendableobj.release();
    }


    return true;
}
// ------------------------------------ //
DLLEXPORT bool SendableEntitySerializer::ApplyUpdateFromPacket(BaseObject* targetobject,
    int ticknumber, int referencetick, sf::Packet &packet)
{

    BaseSendableEntity* asbase = dynamic_cast<BaseSendableEntity*>(targetobject);

    if(!asbase)
        return false;

    // Get the type //
    int32_t objecttype;

    packet >> objecttype;

    BASESENDABLE_ACTUAL_TYPE sendabletype = static_cast<BASESENDABLE_ACTUAL_TYPE>(objecttype);
    
    // Check does the type match //
    if(!asbase->GetSendableType() == sendabletype)
        return false;
    
    return asbase->LoadUpdateFromPacket(packet, ticknumber, referencetick);
}
// ------------------------------------ //
DLLEXPORT bool SendableEntitySerializer::IsObjectTypeCorrect(BaseObject* object) const{

    return dynamic_cast<BaseSendableEntity*>(object) ? true: false;
}

