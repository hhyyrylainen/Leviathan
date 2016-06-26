// ------------------------------------ //
#include "SendableEntitySerializer.h"

#include "../../Utility/Convert.h"
#include "../Components.h"
#include "../../Handlers/ObjectLoader.h"
#include "../CommonStateObjects.h"
#include "../../Networking/NetworkClientInterface.h"
// TODO: remove
#include "../../Networking/ConnectionInfo.h"
using namespace Leviathan;
using namespace std;
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

    switch(sendable.SendableHandleType){
        case SENDABLE_TYPE_BRUSH:
        {
            try{
                auto& box = world->GetComponent<BoxGeometry>(id);
                auto& pos = world->GetComponent<Position>(id);
                auto& physics = world->GetComponent<Physics>(id);
                auto& rendernode = world->GetComponent<RenderNode>(id);

                packet << box.Material << box.Sizes << pos._Position << pos._Orientation <<
                    physics.AppliedPhysicalMaterial << physics.Mass << rendernode.Hidden;

                return true;
                
            } catch(const NotFound&){

                throw InvalidType("Entity is missing required component");
            }
        }
        break;
        case SENDABLE_TYPE_PROP:
        {
            try{
                auto& model = world->GetComponent<Model>(id);
                auto& pos = world->GetComponent<Position>(id);
                auto& physics = world->GetComponent<Physics>(id);
                auto& rendernode = world->GetComponent<RenderNode>(id);

                packet << model.ModelFile << pos._Position << pos._Orientation <<
                    physics.AppliedPhysicalMaterial << rendernode.Hidden;

                return true;
                
            } catch(const NotFound&){

                throw InvalidType("Entity is missing required component");
            }
        }
        break;
        case SENDABLE_TYPE_TRACKCONTROLLER:
        {
            try{
                auto& track = world->GetComponent<TrackController>(id);
                auto& parent = world->GetComponent<Parent>(id);
                auto& positions = world->GetComponent<PositionMarkerOwner>(id);

                packet << static_cast<int32_t>(track.ReachedNode) <<
                    track.NodeProgress << track.ChangeSpeed <<
                    track.ForceTowardsPoint;

                parent.AddDataToPacket(packet);

                positions.AddDataToPacket(packet);
                
                return true;
                
            } catch(const NotFound&){

                throw InvalidType("Entity is missing required component");
            }
        }
        break;

        default:
            throw Exception("Unimplemented Sendable serializer type");
    }
}
// ------------------------------------ //
DLLEXPORT bool SendableEntitySerializer::DeserializeWholeEntityFromPacket(GameWorld* world,
    Lock &worldlock, ObjectID id, int32_t serializetype, sf::Packet &packet)
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

    // Create a sendable entity //
    // But since we are on the client add Received instead of Sendable

    auto matchtype = static_cast<SENDABLE_TYPE>(packetstype);

    switch(matchtype){
        case SENDABLE_TYPE_BRUSH:
        {
            string material;
            Float3 sizes;
            Float3 pos;
            Float4 rot;
            int physicalid;
            float mass;
            bool hidden;

            packet >> material >> sizes >> pos >> rot >> physicalid >> mass >> hidden;

            if(!packet){

                Logger::Get()->Error("SendableEntitySerializer: failed to unserialize brush, "
                    "invalid packet");
                return true;
            }

            if(!ObjectLoader::LoadNetworkBrush(world, worldlock, id, material, sizes, mass,
                    physicalid, {pos, rot}, hidden))
            {

                Logger::Get()->Warning("SendableSerializer: failed to create a brush");
            }

            return true;
        }
        case SENDABLE_TYPE_PROP:
        {
            string modelname;
            Float3 pos;
            Float4 rot;
            int physicalid;
            bool hidden;

            packet >> modelname >> pos >> rot >> physicalid >> hidden;

            if(!packet){

                Logger::Get()->Error("SendableEntitySerializer: failed to unserialize prop, "
                    "invalid packet");
                return true;
            }

            if(!ObjectLoader::LoadNetworkProp(world, worldlock, id, modelname, physicalid,
                    {pos, rot}, hidden))
            {

                Logger::Get()->Warning("SendableSerializer: failed to create a prop");
            }

            return true;
        }
        case SENDABLE_TYPE_TRACKCONTROLLER:
        {
            int reachednode;
            float nodeprogress;
            float changespeed;
            float forcetowardspoint;

            packet >> reachednode >> nodeprogress >> changespeed >>
                forcetowardspoint;

            const auto parentdata = Parent::LoadDataFromPacket(packet);

            const auto positiondata = PositionMarkerOwner::LoadDataFromPacket(packet);

            if(!packet){

                Logger::Get()->Error("SendableEntitySerializer: failed to unserialize "
                    "track controller invalid packet");
                return true;
            }

            if(!ObjectLoader::LoadNetworkTrackController(world, worldlock, id, reachednode,
                    nodeprogress, changespeed, forcetowardspoint, parentdata, positiondata))
            {

                Logger::Get()->Warning("SendableSerializer: failed to create a track controller");
            }

            return true;
        }
        default:
            // Unknown type
            return false;
    }

    return true;
}
// ------------------------------------ //
DLLEXPORT bool SendableEntitySerializer::VerifyAndFillReceivedState(Received* received,
    int ticknumber, int referencetick, shared_ptr<ObjectDeltaStateData> receivedstate)
{
    if(!receivedstate){

        Logger::Get()->Error("SendableEntitySerializer: invalid packet, failed to create "
            "state object");
        
        return false;
    }

    // Store for interpolation //
    // Skip if not newer than any //
    if(received->ClientStateBuffer.size() != 0){

        bool newer = false;
        bool filled = false;
            
        for(auto& obj : received->ClientStateBuffer){

            // Fill data from the reference tick to make this update packet as complete as
            // possible
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
            for(auto& obj : received->ClientStateBuffer){

                if(obj.Tick < referencetick){

                    olderexist = true;
                    break;
                }
            }

            if(!olderexist){

                Logger::Get()->Warning("Entity old state "+Convert::ToString(referencetick)+
                    " is no longer in memory, too small buffer");
            }
        }
            
    } else {

        // No stored states, must be newer //
        // Also no need to fill missing data as only the updated values should be in the packet //
    }

    return true;
}

DLLEXPORT bool SendableEntitySerializer::ApplyUpdateFromPacket(GameWorld* world, Lock &worldlock,
    ObjectID targetobject, int ticknumber, int referencetick, sf::Packet &packet)
{

    Received* received;
    
    try{
        received = &world->GetComponent<Received>(targetobject);
    } catch(...){

        // targetobject is invalid type for us //
        Logger::Get()->Error("SendableEntitySerializer: target object has no Received component");
        return false;
    }
    
    // Get the type //
    int32_t objecttype;

    packet >> objecttype;

    if(!packet){

        Logger::Get()->Error("SendableEntitySerializer: invalid packet, no valid type");
        return false;
    }

    auto sendabletype = static_cast<SENDABLE_TYPE>(objecttype);
    
    // Check does the type match //
    if(received->SendableHandleType != sendabletype){

        Logger::Get()->Error("SendableEntitySerializer: packet doesn't match entity type, "
            +Convert::ToString(sendabletype)+" != "+
            Convert::ToString(received->SendableHandleType));
        
        return false;
    }

    GUARD_LOCK_OTHER(received);
    
    // Create a state object //
    switch(sendabletype){
        case SENDABLE_TYPE_PROP:
        case SENDABLE_TYPE_BRUSH:
        {
            auto state = make_shared<PositionDeltaState>(ticknumber, packet);

            if(!VerifyAndFillReceivedState(received, ticknumber, referencetick, state)){

                // Should only get here if it isn't older than any //
                DEBUG_BREAK;
                return true;
            }

            // Store the new state in the buffer so that it can be found when interpolating //
            received->ClientStateBuffer.push_back(Received::StoredState(state, state.get(),
                    sendabletype));
        }
        break;
        case SENDABLE_TYPE_TRACKCONTROLLER:
        {
            auto state = make_shared<TrackControllerState>(ticknumber, packet);

            if(!VerifyAndFillReceivedState(received, ticknumber, referencetick, state)){

                // Should only get here if it isn't older than any //
                return true;
            }

            // Store the new state in the buffer so that it can be found when interpolating //
            received->ClientStateBuffer.push_back(Received::StoredState(state, state.get(),
                    sendabletype));
        }
        break;
        default:
        {
            Logger::Get()->Error("SendableSerializer: missing delta state case for "+
                Convert::ToString(sendabletype));
            return false;
        }
    }

    // Interpolations can only happen if more than one state is received
    if(received->ClientStateBuffer.size() > 1)
        received->Marked = true;

    // Send an empty packet on next tick //
    NetworkClientInterface::Get()->MarkForNotifyReceivedStates();

    // TODO: remove debug code
    auto connection = NetworkClientInterface::Get()->GetServerConnection();

    if(connection)
        connection->SendKeepAlivePacket();

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

