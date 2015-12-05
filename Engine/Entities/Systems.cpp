// ------------------------------------ //
#include "Systems.h"

#include "../Networking/NetworkHandler.h"
#include "../Networking/ConnectionInfo.h"
#include "../Networking/NetworkResponse.h"
#include "../Networking/NetworkRequest.h"
#include "CommonStateObjects.h"
#include "GameWorld.h"
using namespace Leviathan;
using namespace std;
// ------------------------------------ //

// ------------------ SendableSystem ------------------ //
DLLEXPORT void SendableSystem::ProcessNode(SendableNode &node, ObjectID nodesobject,
    NodeHolder<SendableNode> &pool, Lock &poollock, GameWorld* world, Lock &worldlock,
    int ticknumber) const
{

    if(!node._Sendable.Marked)
        return;

    // Send updates //
    GUARD_LOCK_OTHER((&node._Sendable));

    // Create current state here as one or more conections should require it //
    shared_ptr<ObjectDeltaStateData> curstate;

    switch(node._Sendable.SendableHandleType){
        case SENDABLE_TYPE_PROP:
        case SENDABLE_TYPE_BRUSH:
        {
            curstate = PositionDeltaState::CaptureState(world->GetComponent<Position>(nodesobject),
                ticknumber);
        }
        break;
        case SENDABLE_TYPE_TRACKCONTROLLER:
        {
            curstate = TrackControllerState::CaptureState(world->GetComponent<TrackController>(
                    nodesobject), ticknumber);
        }
        break;
        default:
        {
            Logger::Get()->Error("SendableSystem: processed entity type is invalid, type: "+
                Convert::ToString(node._Sendable.SendableHandleType));
            DEBUG_BREAK;
            return;
        }
    }

    if(!curstate){

        Logger::Get()->Error("SendableSystem: created invalid state for entity, id: "+
            Convert::ToString(nodesobject)+", type: "+Convert::ToString(
                node._Sendable.SendableHandleType));

        node._Sendable.Marked = false;
        return;
    }
            
    auto end = node._Sendable.UpdateReceivers.end();
    for(auto iter = node._Sendable.UpdateReceivers.begin(); iter != end; ){

        std::shared_ptr<Sendable::ActiveConnection> current = *iter;

        // Check is the connection fine //
        std::shared_ptr<ConnectionInfo> safeconnection =
            NetworkHandler::Get()->GetSafePointerToConnection(current->CorrespondingConnection);
        
        // This will be the only function removing closed connections //
        // TODO: do a more global approach to avoid having to lookup connections here
        if(!safeconnection){

            iter = node._Sendable.UpdateReceivers.erase(iter);
            end = node._Sendable.UpdateReceivers.end();

            Logger::Get()->Warning("Sendable invalid corresponding connection");
            // TODO: add a disconnect callback
            continue;
        }

        // Check has some of the packages been received //
        current->CheckReceivedPackets();

        // Prepare the packet //
        std::shared_ptr<sf::Packet> packet = make_shared<sf::Packet>();

        // The first type is used by EntitySerializerManager and the second by
        // the sendable entity serializer
        (*packet) << static_cast<int32_t>(ENTITYSERIALIZEDTYPE_SENDABLE_ENTITY) <<
            static_cast<int32_t>(node._Sendable.SendableHandleType);


        // Do not use last confirmed if it is too old
        int referencetick = -1;

        if(ticknumber < current->LastConfirmedTickNumber + BASESENDABLE_STORED_RECEIVED_STATES -1){

            referencetick = current->LastConfirmedTickNumber;

            // Now calculate a delta update from curstate to the last confirmed state //
            curstate->CreateUpdatePacket(current->LastConfirmedData.get(), *packet.get());

        } else {

            // Data is too old and cannot be used //
            curstate->CreateUpdatePacket(nullptr, *packet.get());

            current->LastConfirmedTickNumber = -1;
            current->LastConfirmedData.reset();
        }

        // Create the final update packet //
        std::shared_ptr<NetworkResponse> updatemesg = make_shared<NetworkResponse>(-1,
            PACKET_TIMEOUT_STYLE_PACKAGESAFTERRECEIVED, 4);

        updatemesg->GenerateEntityUpdateResponse(new
            NetworkResponseDataForEntityUpdate(world->GetID(), nodesobject, ticknumber,
                referencetick, packet));

        Logger::Get()->Write("Sent state: object: "+Convert::ToString(nodesobject)+", tick: "+
            Convert::ToString(ticknumber)+", last confirmed: "+
            Convert::ToString(current->LastConfirmedTickNumber));

        auto sentthing = safeconnection->SendPacketToConnection(updatemesg, 1);

        // Add a callback for success //
        current->AddSentPacket(ticknumber, curstate, sentthing);
        
        ++iter;
    }
    
    node._Sendable.Marked = false;
}


// ------------------ TrackControllerSystem ------------------ //
DLLEXPORT void TrackControllerSystem::ProcessNode(TrackControllerNode &node, ObjectID nodesobject,
    NodeHolder<TrackControllerNode> &pool, Lock &poollock, float timestep, GameWorld* world,
    Lock &worldlock) const
{
    // Advance the state //
    node._TrackController.Update(timestep);

    Float3 pos;
    Float4 rot;
    node._TrackController.GetPositionOnTrack(pos, rot);

    // Apply the position to Parentable objects //
    GUARD_LOCK_OTHER_NAME((&node._Parent), parentlock);


    for(auto&& tuple : node._Parent.Children){

        // Request position //
        auto& physics = world->GetComponent<Physics>(get<0>(tuple));
        auto& position = physics._Position;

        GUARD_LOCK_OTHER_NAME((&physics), physicslock);
        
        // Apply position with the add velocity method //
        Float3 wantedspeed = pos - position._Position;

        wantedspeed = wantedspeed * node._TrackController.ForceTowardsPoint;

        physics.SetVelocity(physicslock, wantedspeed);
            
        // Rotation applying //
        Float4 quaterniontorque = rot.QuaternionMultiply(
            position._Orientation.QuaternionReverse()).Normalize();

        DEBUG_BREAK;
        
        // TODO: verify that this correctly gets the right torque
        Float3 wantedtorque = quaterniontorque.QuaternionToEuler();

        // Set it as the torque //
        physics.SetTorque(physicslock, wantedtorque);
    }
}
