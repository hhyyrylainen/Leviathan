// ------------------------------------ //
#include "Systems.h"

#include "Networking/NetworkHandler.h"
#include "Networking/Connection.h"
#include "Networking/NetworkResponse.h"
#include "Networking/NetworkRequest.h"
#include "CommonStateObjects.h"
#include "GameWorld.h"
using namespace Leviathan;

// ------------------ SendableSystem ------------------ //
DLLEXPORT void HandleNode(ObjectID id, Sendable &obj, GameWorld &world){

    const auto ticknumber = world.GetTickNumber();
    
    // Create current state here as one or more conections should require it //
    shared_ptr<ComponetState> curstate;

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
            DEBUG_BREAK;
            //curstate->CreateUpdatePacket((*iter)->LastConfirmedData.get(), *packet.get());

        } else {

            // Data is too old and cannot be used //
            DEBUG_BREAK;
            //curstate->CreateUpdatePacket(nullptr, *packet.get());

            (*iter)->LastConfirmedTickNumber = -1;
            (*iter)->LastConfirmedData.reset();
        }

        // Create the final update packet //
        ResponseEntityUpdate updatemesg(0, world.GetID(), ticknumber, referencetick, id,
            std::move(updatedata));

        auto sentthing = connection->SendPacketToConnection(updatemesg,
            RECEIVE_GUARANTEE::None);

        // Add a callback for success //
        (*iter)->AddSentPacket(ticknumber, curstate, sentthing);
        
        ++iter;
    }
}
// ------------------------------------ //
DLLEXPORT void ReceivedSystem::Run(std::unordered_map<ObjectID, Received*> &Index,
    GameWorld &world)
{
    const float progress = world.GetTickProgress();
    const auto tick = world.GetTickNumber();
        
    for(auto iter = Index.begin(); iter != Index.end(); ++iter){

        auto& node = *iter->second;

        // Unmarked nodes should have invalid interpolation status
        if(!node.Marked)
            return;
            
        if(!node.LocallyControlled){

            // Interpolate received states //
            float adjustedprogress = progress;
            const Received::StoredState* first;
            const Received::StoredState* second;

            try{
                node.GetServerSentStates(&first, &second, tick, adjustedprogress);
            } catch(const InvalidState&){

                // If not found unmark to avoid running unneeded //
                node.Marked = false;
                continue;
            }
                
            first->Interpolate(second, adjustedprogress);
                
        } else {

            // Send updates to the server //
                
        }
    }
}

