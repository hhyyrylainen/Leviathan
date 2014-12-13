// ------------------------------------ //
#ifndef LEVIATHAN_BASESENDABLEENTITY
#include "BaseSendableEntity.h"
#endif
#include "Entities/Objects/Brush.h"
#include "Entities/Objects/Prop.h"
#include "Entities/Objects/TrackEntityController.h"
using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT Leviathan::BaseSendableEntity::BaseSendableEntity(BASESENDABLE_ACTUAL_TYPE type) :
    SerializeType(type), IsAnyDataUpdated(false)
{


}

DLLEXPORT Leviathan::BaseSendableEntity::~BaseSendableEntity(){

    UpdateReceivers.clear();
}
// ------------------------------------ //
DLLEXPORT BASESENDABLE_ACTUAL_TYPE Leviathan::BaseSendableEntity::GetSendableType() const{

    return SerializeType;
}
// ------------------------------------ //
DLLEXPORT void Leviathan::BaseSendableEntity::SerializeToPacket(sf::Packet &packet){

    packet << static_cast<int32_t>(SerializeType);

    _SaveOwnDataToPacket(packet);
}
// ------------------------------------ //
DLLEXPORT unique_ptr<BaseSendableEntity> Leviathan::BaseSendableEntity::UnSerializeFromPacket(sf::Packet &packet,
    GameWorld* world, int id)
{

    int32_t packetstype;
    if(!(packet >> packetstype)){

        return nullptr;
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
            unique_ptr<Entity::Brush> tmpobj(new Entity::Brush(hidden, world, id));

            if(!tmpobj->_LoadOwnDataFromPacket(packet)){

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
            unique_ptr<Entity::Prop> tmpobj(new Entity::Prop(hidden, world, id));

            if(!tmpobj->_LoadOwnDataFromPacket(packet)){

                Logger::Get()->Warning("BaseSendableEntity: failed to Init Prop from network packet");
                return nullptr;
            }

            return move(unique_ptr<BaseSendableEntity>(dynamic_cast<BaseSendableEntity*>(tmpobj.release())));
        }
        case BASESENDABLE_ACTUAL_TYPE_TRACKENTITYCONTROLLER:
        {
            unique_ptr<Entity::TrackEntityController> tmpobj(new Entity::TrackEntityController(id, world));

            if(!tmpobj->_LoadOwnDataFromPacket(packet)){

                Logger::Get()->Warning("BaseSendableEntity: failed to Init TrackEntityController from network packet");
                return nullptr;
            }

            return move(unique_ptr<BaseSendableEntity>(dynamic_cast<BaseSendableEntity*>(tmpobj.release())));
        }
        default:
            // Unknown type
            return nullptr;
    }
}
// ------------------------------------ //
DLLEXPORT void Leviathan::BaseSendableEntity::AddConnectionToReceivers(ConnectionInfo* receiver){

    GUARD_LOCK_THIS_OBJECT();

    UpdateReceivers.push_back(make_shared<SendableObjectConnectionUpdated>(this, receiver));
}

// ------------------------------------ //
DLLEXPORT void Leviathan::BaseSendableEntity::SendUpdatesToAllClients(){

    GUARD_LOCK_THIS_OBJECT();

    // Return if none could want any updates //
    if(!IsAnyDataUpdated)
        return;

    // Create current state here as one or more conections should require it //
    auto curstate = CaptureState();
    
    auto end = UpdateReceivers.end();
    for(auto iter = UpdateReceivers.begin(); iter != end; ++iter){

        // First check does this particular connection need an update //
        if(!(*iter)->DataUpdatedAfterSending){

            ++iter;
            continue;
        }

        shared_ptr<sf::Packet> packet = make_shared<sf::Packet>();

        // Prepare the packet //
        // The first type is used by EntitySerializerManager and the second by the sendable entity serializer
        (*packet) << static_cast<int32_t>(ENTITYSERIALIZEDTYPE_SENDABLE_ENTITY) << static_cast<int32_t>(SerializeType);
        
        // Now calculate a delta update from curstate to the last confirmed state //
        curstate->CreateUpdatePacket((*iter)->LastConfirmedData.get(), *packet.get());
        
        // Check is the connection fine //
        shared_ptr<ConnectionInfo> safeconnection = NetworkHandler::Get()->GetSafePointerToConnection(
            (*iter)->CorrespondingConnection);
        
        // This will be the only function removing closed connections //
        if(!safeconnection){

            iter = UpdateReceivers.erase(iter);
            // TODO: add a disconnect callback
            continue;
        }

        // Create the final update packet //
        shared_ptr<NetworkResponse> updatemesg = make_shared<NetworkResponse>(-1,
            PACKAGE_TIMEOUT_STYLE_PACKAGESAFTERRECEIVED, 4);

        updatemesg->GenerateEntityUpdateResponse(new NetworkResponseDataForEntityUpdate(OwnedByWorld->GetID(),
                GetID(), packet));

        safeconnection->SendPacketToConnection(updatemesg, 2);

        ++iter;
    }
    
    // After sending every connection is up to date //
    IsAnyDataUpdated = false;
}


// ------------------ SendableObjectConnectionUpdated ------------------ //
DLLEXPORT Leviathan::SendableObjectConnectionUpdate::SendableObjectConnectionUpdated(BaseSendableEntity* getstate,
    ConnectionInfo* connection) :
    CorrespondingConnection(connection), DataUpdatedAfterSending(false), LastConfirmedData(getstate->CaptureState())
{


}
// ------------------ ObjectDeltaStateData ------------------ //
DLLEXPORT Leviathan::ObjectDeltaStateData::~ObjectDeltaStateData(){

}

