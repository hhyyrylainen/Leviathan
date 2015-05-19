// ------------------------------------ //
#include "Systems.h"

#include "../Networking/NetworkHandler.h"
#include "../Networking/ConnectionInfo.h"
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

    DEBUG_BREAK;

    switch(node._Sendable.SendableHandleType){
        case SENDABLE_TYPE_BRUSH:
        {

        }
        break;
        case SENDABLE_TYPE_PROP:
        {

        }
        break;
        case SENDABLE_TYPE_TRACKCONTROLLER:
        {

        }
        break;
    }
            
    auto end = node._Sendable.UpdateReceivers.end();
    for(auto iter = node._Sendable.UpdateReceivers.begin(); iter != end; ){

        std::shared_ptr<sf::Packet> packet = make_shared<sf::Packet>();

        // Prepare the packet //
        // The first type is used by EntitySerializerManager and the second by
        // the sendable entity serializer
        (*packet) << static_cast<int32_t>(ENTITYSERIALIZEDTYPE_SENDABLE_ENTITY) <<
            static_cast<int32_t>(node._Sendable.SendableHandleType);
        
        // Now calculate a delta update from curstate to the last confirmed state //
        curstate->CreateUpdatePacket((*iter)->LastConfirmedData.get(), *packet.get());

        
        // Check is the connection fine //
        std::shared_ptr<ConnectionInfo> safeconnection =
            NetworkHandler::Get()->GetSafePointerToConnection(
                (*iter)->CorrespondingConnection);
        
        // This will be the only function removing closed connections //
        // TODO: do a more global approach to avoid having to lookup connections here
        if(!safeconnection){

            iter = node._Sendable.UpdateReceivers.erase(iter);
            end = node._Sendable.UpdateReceivers.end();
            
            // TODO: add a disconnect callback
            continue;
        }

        // Create the final update packet //
        std::shared_ptr<NetworkResponse> updatemesg = make_shared<NetworkResponse>(-1,
            PACKET_TIMEOUT_STYLE_PACKAGESAFTERRECEIVED, 4);

        updatemesg->GenerateEntityUpdateResponse(new
            NetworkResponseDataForEntityUpdate(world->GetID(), nodesobject, ticknumber,
                (*iter)->LastConfirmedTickNumber, packet));

        auto senthing = safeconnection->SendPacketToConnection(updatemesg, 1);

        // Add a callback for success //
        senthing->SetCallback(std::bind(
                &Sendable::ActiveConnection::OnPacketFinalized, (*iter),
                ticknumber, curstate, placeholders::_1, placeholders::_2));
        
        ++iter;
    }
    
    node._Sendable.Marked = false;
}



