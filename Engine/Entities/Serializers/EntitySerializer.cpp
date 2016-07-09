// ------------------------------------ //
#include "EntitySerializer.h"

#include "Utility/Convert.h"
#include "Entities/GameWorld.h"

using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT bool EntitySerializer::CreatePacketForConnection(GameWorld* world,
    Lock &worldlock, ObjectID id, Sendable &sendable, sf::Packet &packet,
    Connection &connection)
{
    DEBUG_BREAK;

    // TODO: send all the components 
    
    return true;
}
// ------------------------------------ //
DLLEXPORT bool EntitySerializer::DeserializeWholeEntityFromPacket(GameWorld* world,
    Lock &worldlock, ObjectID id, sf::Packet &packet)
{
    DEBUG_BREAK;

    // TODO: load all components
    
    
    return true;
}
// ------------------------------------ //
DLLEXPORT bool EntitySerializer::VerifyAndFillReceivedState(Received* received,
    int ticknumber, int referencetick, std::shared_ptr<ComponentState> receivedstate)
{
    if(!receivedstate){

        Logger::Get()->Error("EntitySerializer: invalid packet, failed to create "
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
                DEBUG_BREAK;
                //receivedstate->FillMissingData(*obj.DeltaData);
                
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

DLLEXPORT bool EntitySerializer::ApplyUpdateFromPacket(GameWorld* world,
    Lock &worldlock, ObjectID targetobject, int ticknumber, int referencetick,
    sf::Packet &packet)
{

    Received* received;
    
    try{
        received = &world->GetComponent<Received>(targetobject);
    } catch(...){

        // targetobject is invalid type for us //
        Logger::Get()->Error("EntitySerializer: target object has no "
            "Received component");
        return false;
    }

    // TODO: load sub update states per component
    
    
    // if(!VerifyAndFillReceivedState(received, ticknumber, referencetick, state)){

    //     // Should only get here if it isn't older than any //
    //     DEBUG_BREAK;
    //     return true;
    // }

    // Interpolations can only happen if more than one state is received
    if(received->ClientStateBuffer.size() > 1)
        received->Marked = true;

    // Send an empty packet on next tick //
    //world->MarkForNotifyReceivedStates();

    return true;
}

// ------------------------------------ //

