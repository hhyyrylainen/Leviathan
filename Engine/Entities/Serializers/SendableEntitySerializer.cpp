// ------------------------------------ //
#include "SendableEntitySerializer.h"

#include "../../Utility/Convert.h"
#include "../Components.h"
#include "../../Handlers/ObjectLoader.h"
#include "../CommonStateObjects.h"
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

    DEBUG_BREAK;
    
    switch(sendable.SendableHandleType){
        case SENDABLE_TYPE_BRUSH:
        {
            try{
                auto& box = world->GetComponent<BoxGeometry>(id);
                auto& pos = world->GetComponent<Position>(id);
                auto& physics = world->GetComponent<Physics>(id);

                packet << box.Material << box.Sizes << pos._Position << pos._Orientation <<
                    physics.AppliedPhysicalMaterial << physics.Mass;

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

                packet << model.ModelFile << pos._Position << pos._Orientation <<
                    physics.AppliedPhysicalMaterial;

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

                packet << track.ReachedNode << track.NodeProgress << track.ChangeSpeed <<
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

            packet >> material >> sizes >> pos >> rot >> physicalid >> mass;

            if(!ObjectLoader::LoadNetworkBrush(world, worldlock, id, material, sizes, mass,
                    physicalid, {pos, rot}))
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

            packet >> modelname >> pos >> rot >> physicalid;

            if(!ObjectLoader::LoadNetworkProp(world, worldlock, id, modelname, physicalid,
                    {pos, rot}))
            {

                Logger::Get()->Warning("SendableSerializer: failed to create a brush");
            }

            return true;
        }
        case SENDABLE_TYPE_TRACKCONTROLLER:
        {

            
            DEBUG_BREAK;
        }
        default:
            // Unknown type
            return nullptr;
    }

    return true;
}
// ------------------------------------ //
DLLEXPORT bool SendableEntitySerializer::ApplyUpdateFromPacket(GameWorld* world, Lock &worldlock,
    ObjectID targetobject, int ticknumber, int referencetick, sf::Packet &packet)
{

    Received* received;
    
    try{
        received = &world->GetComponent<Received>(targetobject);
    } catch(...){

        // targetobject is invalid type for us //
        return false;
    }
    
    // Get the type //
    int32_t objecttype;

    packet >> objecttype;

    auto sendabletype = static_cast<SENDABLE_TYPE>(objecttype);
    
    // Check does the type match //
    if(received->SendableHandleType == sendabletype)
        return false;

    shared_ptr<ObjectDeltaStateData> receivedstate;
    
    // Create a state object //
    switch(sendabletype){
        case SENDABLE_TYPE_PROP:
        case SENDABLE_TYPE_BRUSH:
        {
            DEBUG_BREAK;
        }
        break;
        case SENDABLE_TYPE_TRACKCONTROLLER:
        {
            DEBUG_BREAK;
        }
        break;
        default:
        {
            Logger::Get()->Error("SendableSerializer: missing delta state case for "+
                Convert::ToString(sendabletype));
            return false;
        }
    }

    if(!receivedstate){

        return false;
    }

    // Store for interpolation //
    GUARD_LOCK_OTHER(received);

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

            Logger::Get()->Warning("Entity old state "+Convert::ToString(referencetick)+
                " is no longer in memory");
        }
            
    } else {

        // No stored states, must be newer //
        // Also no need to fill missing data as only the updated values should be in the packet //
    }

    // Store the new state in the buffer so that it can be found when interpolating //
    received->ClientStateBuffer.push_back(Received::StoredState(receivedstate));

    // Interpolations can only happen if more than one state is received
    if(received->ClientStateBuffer.size() > 1)
        received->Marked = true;

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

