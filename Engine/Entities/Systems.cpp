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
DLLEXPORT void HandleNode(ObjectID id, Sendable &obj, GameWorld &world){

    const auto ticknumber = world.GetTickNumber();
    
    // Create current state here as one or more conections should require it //
    shared_ptr<ObjectDeltaStateData> curstate;

    DEBUG_BREAK;
    
    if(!curstate){

        Logger::Get()->Error("SendableSystem: created invalid state for entity, id: "+
            Convert::ToString(id));

        obj.Marked = false;
        return;
    }
    
    for(auto iter = obj.UpdateReceivers.begin(); iter != obj.UpdateReceivers.end(); ){

        auto connection = (*iter)->CorrespondingConnection;
        
        // Find the receiver from the world //
        // And fail if not found or the connection is not open
        if(!world.IsConnectionInWorld(connection) || !connection.IsOpen()){

            // Connection no longer updated for world //
            iter = obj.UpdateReceivers.erase(iter);
            continue;
        }

        // Check has some of the packets been received //
        (*iter)->CheckReceivedPackets();

        // Prepare the packet //
        sf::Packet updatedata;

        // Count the components that have states //
        DEBUG_BREAK;

        // Do not use last confirmed if it is too old
        int referencetick = -1;

        if(ticknumber < (*iter)->LastConfirmedTickNumber +
            BASESENDABLE_STORED_RECEIVED_STATES -1)
        {
            referencetick = (*iter)->LastConfirmedTickNumber;

            // Now calculate a delta update from curstate to the last confirmed state //
            curstate->CreateUpdatePacket((*iter)->LastConfirmedData.get(), *packet.get());

        } else {

            // Data is too old and cannot be used //
            curstate->CreateUpdatePacket(nullptr, *packet.get());

            (*iter)->LastConfirmedTickNumber = -1;
            (*iter)->LastConfirmedData.reset();
        }

        // Create the final update packet //
        ResponseEntityUpdate updatemesg(0, world.GetID(), ticknumber, referencetick, id,
            updatedata);

        auto sentthing = connection->SendPacketToConnection(updatemesg, 1);

        // Add a callback for success //
        (*iter)->AddSentPacket(ticknumber, curstate, sentthing);
        
        ++iter;
    }
}
// ------------------------------------ //


