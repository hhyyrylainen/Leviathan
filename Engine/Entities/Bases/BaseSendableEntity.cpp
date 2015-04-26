// ------------------------------------ //
#include "BaseSendableEntity.h"

#include "Entities/Objects/Brush.h"
#include "Entities/Objects/Prop.h"
#include "Entities/Objects/TrackEntityController.h"
#include "Networking/NetworkResponse.h"
#include "Networking/ConnectionInfo.h"
#include "Networking/NetworkHandler.h"
#include "Handlers/ConstraintSerializerManager.h"
#include "Exceptions.h"
using namespace Leviathan;
using namespace std;
// ------------------------------------ //
DLLEXPORT Leviathan::BaseSendableEntity::BaseSendableEntity(BASESENDABLE_ACTUAL_TYPE type) :
    SerializeType(type), IsAnyDataUpdated(false), 

    // Only clients allocate any space to the circular state buffer //
    ClientStateBuffer(NetworkHandler::Get()->GetNetworkType() == NETWORKED_TYPE_CLIENT ?
        BASESENDABLE_STORED_RECEIVED_STATES: 0)
{
    
}

DLLEXPORT Leviathan::BaseSendableEntity::~BaseSendableEntity(){

    GUARD_LOCK();
    UpdateReceivers.clear();
}
// ------------------------------------ //
DLLEXPORT BASESENDABLE_ACTUAL_TYPE Leviathan::BaseSendableEntity::GetSendableType() const{

    return SerializeType;
}
// ------------------------------------ //
DLLEXPORT void Leviathan::BaseSendableEntity::SerializeToPacket(sf::Packet &packet){
    GUARD_LOCK();
    
    packet << static_cast<int32_t>(SerializeType);

    _SaveOwnDataToPacket(packet);
}
// ------------------------------------ //
DLLEXPORT std::unique_ptr<BaseSendableEntity> Leviathan::BaseSendableEntity::UnSerializeFromPacket(sf::Packet &packet,
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
            std::unique_ptr<Entity::Brush> tmpobj(new Entity::Brush(hidden, world, id));

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
            std::unique_ptr<Entity::Prop> tmpobj(new Entity::Prop(hidden, world, id));

            if(!tmpobj->_LoadOwnDataFromPacket(packet)){

                Logger::Get()->Warning("BaseSendableEntity: failed to Init Prop from network packet");
                return nullptr;
            }

            return move(unique_ptr<BaseSendableEntity>(dynamic_cast<BaseSendableEntity*>(tmpobj.release())));
        }
        case BASESENDABLE_ACTUAL_TYPE_TRACKENTITYCONTROLLER:
        {
            std::unique_ptr<Entity::TrackEntityController> tmpobj(new Entity::TrackEntityController(id, world));

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

    GUARD_LOCK();

    const int tick = OwnedByWorld ? OwnedByWorld->GetTickNumber(): -1;

    UpdateReceivers.push_back(make_shared<SendableObjectConnectionUpdate>(this, receiver, tick));
}
// ------------------------------------ //
DLLEXPORT void Leviathan::BaseSendableEntity::SendUpdatesToAllClients(int ticknumber){

    GUARD_LOCK();

    // Return if none could want any updates //
    if(!IsAnyDataUpdated)
        return;

    // Create current state here as one or more conections should require it //
    auto curstate = CaptureState(ticknumber);
    
    auto end = UpdateReceivers.end();
    for(auto iter = UpdateReceivers.begin(); iter != end; ){

        // Currently all active connections will receive all updates //

        std::shared_ptr<sf::Packet> packet = make_shared<sf::Packet>();

        // Prepare the packet //
        // The first type is used by EntitySerializerManager and the second by the sendable entity serializer
        (*packet) << static_cast<int32_t>(ENTITYSERIALIZEDTYPE_SENDABLE_ENTITY) << static_cast<int32_t>(SerializeType);
        
        // Now calculate a delta update from curstate to the last confirmed state //
        curstate->CreateUpdatePacket((*iter)->LastConfirmedData.get(), *packet.get());
        
        // Check is the connection fine //
        std::shared_ptr<ConnectionInfo> safeconnection = NetworkHandler::Get()->GetSafePointerToConnection(
            (*iter)->CorrespondingConnection);
        
        // This will be the only function removing closed connections //
        // TODO: do a more global approach to avoid having to lookup connections here
        if(!safeconnection){

            iter = UpdateReceivers.erase(iter);
            end = UpdateReceivers.end();
            
            // TODO: add a disconnect callback
            continue;
        }

        // Create the final update packet //
        std::shared_ptr<NetworkResponse> updatemesg = make_shared<NetworkResponse>(-1,
            PACKET_TIMEOUT_STYLE_PACKAGESAFTERRECEIVED, 4);

        updatemesg->GenerateEntityUpdateResponse(new NetworkResponseDataForEntityUpdate(OwnedByWorld->GetID(),
                GetID(), ticknumber, (*iter)->LastConfirmedTickNumber, packet));

        auto senthing = safeconnection->SendPacketToConnection(updatemesg, 1);

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
void Leviathan::BaseSendableEntity::_MarkDataUpdated(Lock &guard){
    VerifyLock(guard);
    
    // Mark all active receivers as needing an update //
    IsAnyDataUpdated = true;

    if(UpdateReceivers.empty())
        return;
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::BaseSendableEntity::LoadUpdateFromPacket(sf::Packet &packet, int ticknumber,
    int referencetick)
{

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
DLLEXPORT void BaseSendableEntity::GetServerSentStates(shared_ptr<ObjectDeltaStateData> &first,
    std::shared_ptr<ObjectDeltaStateData> &second, int tick, float &progress) const
{
    bool firstfound = false;
    int secondfound = 0;

    {
        GUARD_LOCK();
    
        for(auto& obj : ClientStateBuffer){

            if(obj.Tick == tick){
            
                // This is the first state //
                first = obj.DeltaData;

                firstfound = true;
                continue;
            }

            // For this to be found the client should be around 50-100 milliseconds in the past
            if(obj.Tick > tick && (secondfound == 0 || obj.Tick-tick < secondfound)){

                // The second state //
                second = obj.DeltaData;
            
                secondfound = obj.Tick-tick;
                continue;
            }
        }
    }

    if(!firstfound || secondfound == 0){

        throw InvalidState("No stored server states around tick");
    }

    // Adjust progress //
    if(secondfound > 1){

        const float mspassed = TICKSPEED*progress;
        progress = mspassed / (TICKSPEED*secondfound);
    }
}
// ------------------------------------ //
void Leviathan::BaseSendableEntity::_SendNewConstraint(BaseConstraintable* us, BaseConstraintable* other,
    Entity::BaseConstraint* constraint)
{
    GUARD_LOCK();

    if(UpdateReceivers.empty())
        return;

    GUARD_LOCK_OTHER_NAME(constraint, guard2);

    auto custompacketdata = ConstraintSerializerManager::Get()->SerializeConstraintData(constraint);

    if(!custompacketdata){

        Logger::Get()->Warning("BaseSendableEntity: failed to send constraint, type: "+Convert::ToString(
                static_cast<int>(constraint->GetType())));
        return;
    }
    
    // Gather all the other info //
    int obj1 = GetID();

    // The second object might be NULL so make sure not to segfault here //
    int obj2 = other ? other->GetID(): -1;

    auto packet = make_shared<NetworkResponse>(-1, PACKET_TIMEOUT_STYLE_TIMEDMS, 1000);
    
    // Wrap everything up and send //
    packet->GenerateEntityConstraintResponse(new NetworkResponseDataForEntityConstraint(OwnedByWorld->GetID(),
            obj1, obj2, true, constraint->GetType(), custompacketdata));

    auto end = UpdateReceivers.end();
    for(auto iter = UpdateReceivers.begin(); iter != end; ++iter){

        auto connectionptr = NetworkHandler::Get()->GetSafePointerToConnection((*iter)->CorrespondingConnection);

        if(!connectionptr)
            break;
        
        connectionptr->SendPacketToConnection(packet, 12);
    }
}
// ------------------ SendableObjectConnectionUpdated ------------------ //
DLLEXPORT Leviathan::SendableObjectConnectionUpdate::SendableObjectConnectionUpdate(BaseSendableEntity* getstate,
    ConnectionInfo* connection, int tick) :
    CorrespondingConnection(connection),
    // TODO: make these this entity global to avoid capturing bunch of states whenever a new player
    // connects
    LastConfirmedData(getstate->CaptureState(tick)),
    LastConfirmedTickNumber(-1)
{


}

DLLEXPORT void Leviathan::SendableObjectConnectionUpdate::SucceedOrFailCallback(int ticknumber,
    std::shared_ptr<ObjectDeltaStateData> state, bool succeeded, SentNetworkThing &us)
{
    if(!succeeded)
        return;

    Lock lock(CallbackMutex);
    
    if(ticknumber < LastConfirmedTickNumber)
        return;

    LastConfirmedTickNumber = ticknumber;
    LastConfirmedData = state;
}
// ------------------ ObjectDeltaStateData ------------------ //
DLLEXPORT Leviathan::ObjectDeltaStateData::ObjectDeltaStateData(int tick) : Tick(tick){
    
}

DLLEXPORT Leviathan::ObjectDeltaStateData::~ObjectDeltaStateData(){

}
// ------------------ StoredState ------------------ //
StoredState::StoredState(shared_ptr<ObjectDeltaStateData> data) :
    DeltaData(data), Tick(data->Tick)
{

}

StoredState::StoredState(StoredState&& other) :
    DeltaData(move(other.DeltaData)), Tick(other.Tick)
{
    
}

StoredState& StoredState::operator=(StoredState&& other){

    DeltaData = move(other.DeltaData);
    Tick = other.Tick;
}

