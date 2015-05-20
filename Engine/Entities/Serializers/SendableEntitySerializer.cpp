// ------------------------------------ //
#include "SendableEntitySerializer.h"

#include "../../Utility/Convert.h"
#include "../Components.h"
using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT SendableEntitySerializer::SendableEntitySerializer() :
    BaseEntitySerializer(ENTITYSERIALIZEDTYPE_SENDABLE_ENTITY)
{

    
}

DLLEXPORT SendableEntitySerializer::~SendableEntitySerializer(){


}
// ------------------------------------ //
DLLEXPORT bool SendableEntitySerializer::CreatePacketForConnection(GameWorld* world,
    Lock &worldlock, ObjectID id, Sendable &sendable, sf::Packet &packet,
    ConnectionInfo* connectionptr)
{
    // Check type //
    if(!IsObjectTypeCorrect(sendable))
        return false;

    // The type is correct, so we should always return true after this //
    
    // First add the type variable and then data according to the serailize type
    packet << Type << sendable.SendableHandleType;

    GUARD_LOCK_OTHER((&sendable));
    
    // Add the connection to the known ones (this is before the serialize to make sure that
    // all updates after serializing get sent)
    sendable.AddConnectionToReceivers(guard, connectionptr);

    DEBUG_BREAK;
    
    switch(sendable.SendableHandleType){

        default:
            throw Exception("Unimplemented Sendable serializer type");
    }
    
    return true;
}
// ------------------------------------ //
DLLEXPORT bool SendableEntitySerializer::DeserializeWholeEntityFromPacket(GameWorld* world,
    Lock &worldlock, ObjectID id, int32_t serializetype, sf::Packet &packet, int objectid)
{
    // Verify that the type matches //
    if(serializetype != Type){

        Logger::Get()->Error("SendableEntitySerializer: passed wrong serializetype to "
            "unserialize, Our: "+Convert::ToString(Type)+", got: "+
            Convert::ToString(serializetype));
        return false;
    }
    
    int32_t packetstype;
    if(!(packet >> packetstype)){

        return false;
    }

    BASESENDABLE_ACTUAL_TYPE matchtype = static_cast<BASESENDABLE_ACTUAL_TYPE>(packetstype);

        switch(matchtype){
            case BASESENDABLE_ACTUAL_TYPE_BRUSH:
            {
                // This is hopefully written by the Brush _SaveOwnData
                bool hidden;

                if(!(packet >> hidden)){

                    return nullptr;
                }
            
                // Create a brush and apply the packet to it //
                std::unique_ptr<Entity::Brush> tmpobj(new Entity::Brush(hidden, world, id));

                GUARD_LOCK_OTHER(tmpobj);
                if(!tmpobj->_LoadOwnDataFromPacket(guard, packet)){

                    Logger::Get()->Warning("BaseSendableEntity: failed to Init Brush from network packet");
                    return nullptr;
                }
            
                return move(unique_ptr<BaseSendableEntity>(dynamic_cast<BaseSendableEntity*>(tmpobj.release())));
            }
            case BASESENDABLE_ACTUAL_TYPE_PROP:
            {
                // This is hopefully written by the Prop _SaveOwnData
                bool hidden;

                if(!(packet >> hidden)){

                    return nullptr;
                }
            
                // Create a brush and apply the packet to it //
                std::unique_ptr<Entity::Prop> tmpobj(new Entity::Prop(hidden, world, id));

                GUARD_LOCK_OTHER(tmpobj);
                if(!tmpobj->_LoadOwnDataFromPacket(guard, packet)){

                    Logger::Get()->Warning("BaseSendableEntity: failed to Init Prop from network packet");
                    return nullptr;
                }

                return move(unique_ptr<BaseSendableEntity>(dynamic_cast<BaseSendableEntity*>(tmpobj.release())));
            }
            case BASESENDABLE_ACTUAL_TYPE_TRACKENTITYCONTROLLER:
            {
                std::unique_ptr<Entity::TrackEntityController> tmpobj(
                    new Entity::TrackEntityController(id, world));

                GUARD_LOCK_OTHER(tmpobj);
                if(!tmpobj->_LoadOwnDataFromPacket(guard, packet)){

                    Logger::Get()->Warning("BaseSendableEntity: failed to Init "
                        "TrackEntityController from network packet");
                    return nullptr;
                }

                return move(unique_ptr<BaseSendableEntity>(dynamic_cast<BaseSendableEntity*>(
                            tmpobj.release())));
            }
            default:
                // Unknown type
                return nullptr;
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
DLLEXPORT bool SendableEntitySerializer::ApplyUpdateFromPacket(GameWorld* world, Lock &worldlock,
    ObjectID targetobject, int ticknumber, int referencetick, sf::Packet &packet)
{

    try{
        Received& received = world->GetComponent<Received>();
    } catch(...){

        // targetobject is invalid type for us //
        return false;
    }
    
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

    auto receivedstate = CreateStateFromPacket(ticknumber, packet);

    if(!receivedstate){

        return false;
    }

    {
        GUARD_LOCK();

        // Skip if not newer than any //
        if(ClientStateBuffer.size() != 0){

            bool newer = false;
            bool filled = false;
            
            for(auto& obj : ClientStateBuffer){

                // Fill data from the reference tick to make this update packet as complete as possible //
                if(obj.Tick == referencetick){

                    // Add missing values //
                    receivedstate->FillMissingData(*obj.DeltaData);
                
                    filled = true;
                
                    if(newer)
                        break;
                }

                if(obj.Tick < ticknumber){
                
                    newer = true;

                    if(filled)
                        break;
                }
            }
            
            if(!newer)
                return false;

            // If it isn't filled that tells that our buffer is too small //
            // referencetick is invalid when it is -1 and is ignored in that case //
            if(!filled && referencetick != -1){

                bool olderexist = false;

                // TODO: make sure that this doesn't mess with interpolation too badly
                // under reasonable network stress, also this probably should never be missing
                // under normal conditions
                
                // Or that we have missed a single packet //
                for(auto& obj : ClientStateBuffer){

                    if(obj.Tick < referencetick){

                        olderexist = true;
                        break;
                    }
                }

                cout << "Entity old state " << referencetick << " is no longer in memory" << endl;
                
                //if(!olderexist)
                //    throw Exception("ReferenceTick is no longer in memory ClientStateBuffer "
                //        "is too small");
            }
            
        } else {

            // No stored states, must be newer //
            // Also no need to fill missing data as only the updated values should be in the packet //
            
        }

        
        // Store the new state in the buffer so that it can be found when interpolating //
        ClientStateBuffer.push_back(StoredState(receivedstate));
    }

    // Interpolations can only happen if more than one state is received
    if(ClientStateBuffer.size() > 1)
        _OnNewStateReceived();

    return true;
}
// ------------------------------------ //
DLLEXPORT bool SendableEntitySerializer::IsObjectTypeCorrect(Sendable &object) const{

    switch(object.SendableHandleType){
        case SENDABLE_TYPE_TRACKCONTROLLER:
        case SENDABLE_TYPE_PROP:
        case SENDABLE_TYPE_BRUSH:
            return true;
        default:
            return false;
    }
}

