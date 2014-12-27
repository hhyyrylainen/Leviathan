// ------------------------------------ //
#ifndef LEVIATHAN_BASESENDABLEENTITY
#include "BaseSendableEntity.h"
#endif
#include "Entities/Objects/Brush.h"
#include "Entities/Objects/Prop.h"
#include "Entities/Objects/TrackEntityController.h"
#include "Networking/NetworkResponse.h"
#include "Networking/ConnectionInfo.h"
#include "Networking/NetworkHandler.h"
#include "boost/thread/lock_types.hpp"
using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT Leviathan::BaseSendableEntity::BaseSendableEntity(BASESENDABLE_ACTUAL_TYPE type) :
    SerializeType(type), IsAnyDataUpdated(false), LastVerifiedTick(-1),
    // Only clients allocate any space to the circular state buffer //
    ClientStateBuffer(NetworkHandler::Get()->GetNetworkType() == NETWORKED_TYPE_CLIENT ?
        BASESENDABLE_STORED_CLIENT_STATES: 0)
{
    
}

DLLEXPORT Leviathan::BaseSendableEntity::~BaseSendableEntity(){

    GUARD_LOCK_THIS_OBJECT();
    UpdateReceivers.clear();
}
// ------------------------------------ //
DLLEXPORT BASESENDABLE_ACTUAL_TYPE Leviathan::BaseSendableEntity::GetSendableType() const{

    return SerializeType;
}
// ------------------------------------ //
DLLEXPORT void Leviathan::BaseSendableEntity::SerializeToPacket(sf::Packet &packet){
    GUARD_LOCK_THIS_OBJECT();
    
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

    UpdateReceivers.push_back(make_shared<SendableObjectConnectionUpdate>(this, receiver));
}

// ------------------------------------ //
DLLEXPORT void Leviathan::BaseSendableEntity::SendUpdatesToAllClients(int ticknumber){

    GUARD_LOCK_THIS_OBJECT();

    // Return if none could want any updates //
    if(!IsAnyDataUpdated)
        return;

    // Create current state here as one or more conections should require it //
    auto curstate = CaptureState();
    
    auto end = UpdateReceivers.end();
    for(auto iter = UpdateReceivers.begin(); iter != end; ){

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
            end = UpdateReceivers.end();
            
            // TODO: add a disconnect callback
            continue;
        }

        // Create the final update packet //
        shared_ptr<NetworkResponse> updatemesg = make_shared<NetworkResponse>(-1,
            PACKAGE_TIMEOUT_STYLE_PACKAGESAFTERRECEIVED, 4);

        updatemesg->GenerateEntityUpdateResponse(new NetworkResponseDataForEntityUpdate(OwnedByWorld->GetID(),
                GetID(), ticknumber, packet));

        auto senthing = safeconnection->SendPacketToConnection(updatemesg, 2);

        // Add a callback for success //
        senthing->SetCallback(boost::bind(
                &SendableObjectConnectionUpdate::SucceedOrFailCallback, (*iter), ticknumber, curstate,
                _1, _2));
        
        ++iter;
    }
    
    // After sending every connection is up to date //
    IsAnyDataUpdated = false;
}
// ------------------------------------ //
void Leviathan::BaseSendableEntity::_MarkDataUpdated(ObjectLock &guard){
    VerifyLock(guard);
    
    // Mark all active receivers as needing an update //
    IsAnyDataUpdated = true;

    if(UpdateReceivers.empty())
        return;

    auto end = UpdateReceivers.end();
    for(auto iter = UpdateReceivers.begin(); iter != end; ++iter){

        (*iter)->DataUpdatedAfterSending = true;
    }
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::BaseSendableEntity::LoadUpdateFromPacket(sf::Packet &packet, int ticknumber){
    {
        GUARD_LOCK_THIS_OBJECT();
        
        // Ignore if older than latest update //
        if(ticknumber < LastVerifiedTick){

            return true;
        }

        LastVerifiedTick = ticknumber;
    }
    
    // First find an old state for us that is on the same tick //
    shared_ptr<ObjectDeltaStateData> ourold;

    int oldestfound = INT_MAX;
    
    {
        GUARD_LOCK_THIS_OBJECT();
        auto end = ClientStateBuffer.end();
        for(auto iter = ClientStateBuffer.begin(); iter != end; ++iter){
            
            if(iter->Tick < oldestfound)
                oldestfound = iter->Tick;
            
            if(iter->Tick == ticknumber){

                // Found a matching state //
                ourold = iter->State;
                break;
            }
        }
    }
    
    if(!ourold){
        
        // Didn't find an old state //
        // This could be because the object just started moving //

        // This means that the state has already been popped //
        if(oldestfound > ticknumber)
            Logger::Get()->Warning("BaseSendableEntity: coulnd't find our old state for tick number "+
                Convert::ToString(ticknumber));
    }

    // Then we create a state from the packet filling in the blanks from the old state //
    auto receivedstate = CreateStateFromPacket(packet);

    if(!receivedstate)
        return false;

    // Now the implementation checks if we correctly simulated the entity on the client side //
    VerifyOldState(receivedstate.get(), ourold.get(), ticknumber);

    return true;
}
// ------------------------------------ //
DLLEXPORT void Leviathan::BaseSendableEntity::StoreClientSideState(int ticknumber){

    GUARD_LOCK_THIS_OBJECT();

    if(!IsAnyDataUpdated)
        return;
    
    assert(ClientStateBuffer.capacity() != 0 && "StoreClientSideState called on something that isn't a client");

    ClientStateBuffer.push_back(SendableObjectClientState(ticknumber, CaptureState()));
}
// ------------------ SendableObjectConnectionUpdated ------------------ //
DLLEXPORT Leviathan::SendableObjectConnectionUpdate::SendableObjectConnectionUpdate(BaseSendableEntity* getstate,
    ConnectionInfo* connection) :
    CorrespondingConnection(connection), DataUpdatedAfterSending(false), LastConfirmedData(getstate->CaptureState()),
    LastConfirmedTickNumber(-1)
{


}

DLLEXPORT void Leviathan::SendableObjectConnectionUpdate::SucceedOrFailCallback(int ticknumber,
    shared_ptr<ObjectDeltaStateData> state, bool succeeded, SentNetworkThing &us)
{
    if(!succeeded)
        return;

    boost::unique_lock<boost::mutex> lock(CallbackMutex);
    
    if(ticknumber < LastConfirmedTickNumber)
        return;

    LastConfirmedTickNumber = ticknumber;
    LastConfirmedData = state;
}
// ------------------ ObjectDeltaStateData ------------------ //
DLLEXPORT Leviathan::ObjectDeltaStateData::~ObjectDeltaStateData(){

}
// ------------------ SendableObjectClientState ------------------ //
Leviathan::SendableObjectClientState::SendableObjectClientState(int tick, shared_ptr<ObjectDeltaStateData> state) :
    Tick(tick), State(state)
{
    
}
