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
#include "Handlers/ConstraintSerializerManager.h"
#include "Exceptions.h"
#include "Common/Misc.h"
using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT Leviathan::BaseSendableEntity::BaseSendableEntity(BASESENDABLE_ACTUAL_TYPE type) :
    SerializeType(type), IsAnyDataUpdated(false), 

    // Only clients allocate any space to the circular state buffer //
    ClientStateBuffer(NetworkHandler::Get()->GetNetworkType() == NETWORKED_TYPE_CLIENT ?
#ifndef NETWORK_USE_SNAPSHOTS        
        BASESENDABLE_STORED_CLIENT_STATES
#else
        BASESENDABLE_STORED_RECEIVED_STATES
#endif //NETWORK_USE_SNAPSHOTS
        : 0), LastVerifiedTick(-1)
{
#ifdef NETWORK_USE_SNAPSHOTS
    LastQueuedTick = -1;
#endif //NETWORK_USE_SNAPSHOTS
    
    if(NetworkHandler::Get()->GetNetworkType() == NETWORKED_TYPE_CLIENT)
        IsOnClient = true;
}

DLLEXPORT Leviathan::BaseSendableEntity::~BaseSendableEntity(){

    GUARD_LOCK_THIS_OBJECT();
    UpdateReceivers.clear();
}

bool BaseSendableEntity::IsOnClient = false;
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

    const int tick = OwnedByWorld ? OwnedByWorld->GetTickNumber(): -1;

    UpdateReceivers.push_back(make_shared<SendableObjectConnectionUpdate>(this, receiver, tick));
}
// ------------------------------------ //
DLLEXPORT void Leviathan::BaseSendableEntity::SendUpdatesToAllClients(int ticknumber){

    GUARD_LOCK_THIS_OBJECT();

    // Return if none could want any updates //
    if(!IsAnyDataUpdated)
        return;

    // Create current state here as one or more conections should require it //
    auto curstate = CaptureState(ticknumber);
    
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
#ifndef NETWORK_USE_SNAPSHOTS

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
            
            if((*iter)->Tick < oldestfound)
                oldestfound = (*iter)->Tick;
            
            if((*iter)->Tick == ticknumber){

                // Found a matching state //
                ourold = (*iter);
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
    auto receivedstate = CreateStateFromPacket(ticknumber, packet);

    if(!receivedstate)
        return false;

    // Now the implementation checks if we correctly simulated the entity on the client side //
    VerifyOldState(receivedstate.get(), ourold.get(), ticknumber);
#else

    // Here find the last state from which we are interpolating and then request that the implementation
    // starts interpolating

    shared_ptr<ObjectDeltaStateData> laststate;

    auto receivedstate = CreateStateFromPacket(ticknumber, packet);

    {
        GUARD_LOCK_THIS_OBJECT();

        // Skip if older than the last queued one //
        if(ticknumber < LastQueuedTick){

            return true;
        }

        // Skip if not newer than any //
        bool newer = false;

        
        for(auto& obj : ClientStateBuffer){

            if(obj->Tick == LastQueuedTick){
                
                laststate = obj;
            }
        }

        // Store the new state in the buffer so that it can be found when newer ones arrive //
        ClientStateBuffer.push_back(receivedstate);
    }
    

    if(!laststate){

        // The object just started moving //
        QueueInterpolation(NULL, receivedstate, INTERPOLATION_TIME);
        
    } else {

        // The object will need to move from the earlier state //

        const int timeapart = TICKSPEED*(ticknumber-laststate->Tick);

        const int fromlastqueued = TICKSPEED * (ticknumber-LastQueuedTick);

        cout << "Queueing: " << laststate->Tick << " -> " << ticknumber << " in " << timeapart
             << endl;
        
        if(timeapart != fromlastqueued){

            DEBUG_BREAK;
        }

        // We actually only want to interpolate between states that are INTERPOLATION_TIME apart
        // This verifies that the object doesn't stop moving if a single packet is missed
        if(timeapart >= INTERPOLATION_TIME){

            QueueInterpolation(laststate, receivedstate, timeapart-1);
        }
        
        // If we didn't queue it the running out of interpolation steps callback can find the skipped
        // state and interpolate to there
        
    }
    
#endif //NETWORK_USE_SNAPSHOTS

    return true;
}
// ------------------------------------ //
#ifndef NETWORK_USE_SNAPSHOTS
DLLEXPORT void Leviathan::BaseSendableEntity::StoreClientSideState(int ticknumber){

    GUARD_LOCK_THIS_OBJECT();

    if(!IsAnyDataUpdated)
        return;
    
    if(ClientStateBuffer.capacity() != 0)
        throw Exception("StoreClientSideState called on something that isn't a client");

    ClientStateBuffer.push_back(SendableObjectClientState(ticknumber, CaptureState(ticknumber)));
}

DLLEXPORT bool Leviathan::BaseSendableEntity::ReplaceOldClientState(int onticktoreplace,
    shared_ptr<ObjectDeltaStateData> state)
{

    GUARD_LOCK_THIS_OBJECT();

    auto end = ClientStateBuffer.end();
    for(auto iter = ClientStateBuffer.begin(); iter != end; ++iter){

        // Compare ticks to find the one to replace //
        if(iter->Tick == onticktoreplace){

            iter->State = state;
            return true;
        }
    }

    return false;
}
#else

DLLEXPORT void Leviathan::BaseSendableEntity::QueueInterpolation(shared_ptr<ObjectDeltaStateData> from,
    shared_ptr<ObjectDeltaStateData> to, int mstime)
{
    // If we are interpolating from the initial state from is null //
    GUARD_LOCK_THIS_OBJECT();

    
    if(from == nullptr){

        from = CaptureState(0);
        
    } else {

        // The from state needs to have all valid data //
        bool filled = false;

        // This may get expensive quick //
        // Better way could be making all interpolation methods snap back to the old value only if the older contains
        // that value instead of forcing all older states to be full
        for(int targettick = from->Tick-1; targettick > from->Tick-BASESENDABLE_STORED_RECEIVED_STATES; --targettick){
            
            // Allow from to capture values from an earlier state //
            for(auto& obj : ClientStateBuffer){

                if(obj->Tick == targettick){
                
                    if(from->FillMissingData(*obj)){

                        filled = true;
                        goto fillsucceededendlooplable;
                    }

                    break;
                }
            }
        }

fillsucceededendlooplable:
        
        // Last chance to fill with new state //
        if(!filled){

            bool succeeded = from->FillMissingData(*CaptureState(0));
            if(!succeeded)
                throw Exception("coulnd't properly fill a state, even with CaptureState");
        }
    }

    // Set this to the last tick that is included as the end point in a interpolation step
    // So further animations will work
    LastQueuedTick = to->Tick;

    QueuedInterpolationStates.push_back(move(ObjectInterpolation(from, to, mstime)));

    if(QueuedInterpolationStates.size() > 1){

        DEBUG_BREAK;
    }
    
    VerifySendableInterpolation();
}

DLLEXPORT ObjectInterpolation Leviathan::BaseSendableEntity::GetAndPopNextInterpolation(){

    GUARD_LOCK_THIS_OBJECT();
    
    if(QueuedInterpolationStates.empty()){

        _CreateShortInterpolationFromStored(guard);

        // Fail if no new interpolations were found //
        if(QueuedInterpolationStates.empty())
            throw InvalidState("no states in queue");
    }
    
    auto obj = QueuedInterpolationStates.front();
    QueuedInterpolationStates.pop_front();

    ReportInterpolationStatusToInput(obj.Second->Tick, Misc::GetTimeMs64()+obj.Duration);

    return obj;
}

void BaseSendableEntity::_CreateShortInterpolationFromStored(ObjectLock &guard){

    VerifyLock(guard);

    // We need to find any interpolation were Tick > LastQueuedTick //
    shared_ptr<ObjectDeltaStateData> laststate;
    shared_ptr<ObjectDeltaStateData> targetstate;
    
    for(auto& state : ClientStateBuffer){

        if(state->Tick > LastQueuedTick){

            targetstate = state;
        } else if(state->Tick == LastQueuedTick){

            laststate = state;
        }
    }

    if(!targetstate)
        return;
    
    const int timeapart = TICKSPEED * (targetstate->Tick - LastQueuedTick);

    // laststate may be NULL here //
    QueueInterpolation(laststate, targetstate, timeapart);
}

void Leviathan::BaseSendableEntity::ReportInterpolationStatusToInput(int tick, int64_t mstime){
    
    
}
#endif //NETWORK_USE_SNAPSHOTS
// ------------------------------------ //
void Leviathan::BaseSendableEntity::_SendNewConstraint(BaseConstraintable* us, BaseConstraintable* other,
    Entity::BaseConstraint* constraint)
{
    GUARD_LOCK_THIS_OBJECT();

    if(UpdateReceivers.empty())
        return;

    GUARD_LOCK_OTHER_OBJECT_NAME(constraint, guard2);

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

    auto packet = make_shared<NetworkResponse>(-1, PACKAGE_TIMEOUT_STYLE_TIMEDMS, 1000);
    
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
    CorrespondingConnection(connection), DataUpdatedAfterSending(false),
    LastConfirmedData(getstate->CaptureState(tick)),
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
    
#ifdef NETWORK_USE_SNAPSHOTS

    // We want to only mark every packet that is INTERPOLATION_TIME apart //
    // Or if we have missed a interpolation time apart packet
    if(ticknumber % INTERPOLATION_TIME/TICKSPEED == 0 ||
        ticknumber-LastConfirmedTickNumber > INTERPOLATION_TIME/TICKSPEED)
    {
        LastConfirmedTickNumber = ticknumber;
        LastConfirmedData = state;
    }
#else
    LastConfirmedTickNumber = ticknumber;
    LastConfirmedData = state;
#endif //NETWORK_USE_SNAPSHOTS
}
// ------------------ ObjectDeltaStateData ------------------ //
DLLEXPORT Leviathan::ObjectDeltaStateData::ObjectDeltaStateData(int tick) : Tick(tick){
    
}

DLLEXPORT Leviathan::ObjectDeltaStateData::~ObjectDeltaStateData(){

}
// ------------------ ObjectInterpolation ------------------ //
#ifdef NETWORK_USE_SNAPSHOTS

Leviathan::ObjectInterpolation::ObjectInterpolation(shared_ptr<ObjectDeltaStateData> first,
    shared_ptr<ObjectDeltaStateData> second, int duration) :
    First(first), Second(second), Duration(duration)
{
    
}

ObjectInterpolation::ObjectInterpolation(const ObjectInterpolation &other) :
    First(other.First), Second(other.Second), Duration(other.Duration)
{

}

Leviathan::ObjectInterpolation::ObjectInterpolation(ObjectInterpolation &&other) :
    First(move(other.First)), Second(move(other.Second)), Duration(other.Duration)
{
    
}

ObjectInterpolation::ObjectInterpolation() : Duration(-1){
    
}

ObjectInterpolation::~ObjectInterpolation(){

    Second.reset();
    First.reset();
}

ObjectInterpolation& ObjectInterpolation::operator=(const ObjectInterpolation &other){

    Duration = other.Duration;
    First = other.First;
    Second = other.Second;
}

#endif //NETWORK_USE_SNAPSHOTS
