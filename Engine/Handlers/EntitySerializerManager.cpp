// ------------------------------------ //
#include "EntitySerializerManager.h"

#include "Entities/Serializers/BaseEntitySerializer.h"
#include "../Entities/GameWorld.h"
using namespace Leviathan;
using namespace std;
// ------------------------------------ //
DLLEXPORT Leviathan::EntitySerializerManager::EntitySerializerManager(){

    Staticinstance = this;
}

DLLEXPORT Leviathan::EntitySerializerManager::~EntitySerializerManager(){

}

DLLEXPORT EntitySerializerManager* Leviathan::EntitySerializerManager::Get(){

    return Staticinstance;
}

EntitySerializerManager* Leviathan::EntitySerializerManager::Staticinstance = NULL;
// ------------------------------------ //
DLLEXPORT void Leviathan::EntitySerializerManager::Release(){

    GUARD_LOCK();
    Staticinstance = NULL;

    // Delete the registered serializers //
    SAFE_DELETE_VECTOR(Serializers);
}
// ------------------------------------ //
DLLEXPORT void Leviathan::EntitySerializerManager::AddSerializer(BaseEntitySerializer* serializer){
    if(!serializer)
        return;
    
    GUARD_LOCK();

    Serializers.push_back(serializer);
}
// ------------------------------------ //
DLLEXPORT std::unique_ptr<sf::Packet> Leviathan::EntitySerializerManager::CreateInitialEntityMessageFor(
    GameWorld* world, Lock &worldlock, ObjectID id, ConnectionInfo* forwho)
{
    // Setup a packet //
    auto packet = make_unique<sf::Packet>();

    try{
        
        auto& sendable = world->GetComponent<Sendable>(id);

        if(!packet || id == 0)
            return nullptr;
    
        (*packet) << id;

        GUARD_LOCK();
    
        // Find a serializer that can finish it //
        for(size_t i = 0; i < Serializers.size(); i++){

            try{
                if(Serializers[i]->CreatePacketForConnection(world, worldlock, id, sendable,
                        *packet, forwho))
                {
                    return move(packet);
                }
            } catch(const Exception &e){
                
                Logger::Get()->Error("EntitySerializer: correct serializer threw an exception");
                e.PrintToLog();
                DEBUG_BREAK;
            }
        }

        // No serializer was able to do anything //
        return nullptr;

    } catch(const NotFound&){

        Logger::Get()->Error("EntitySerializerManager: trying to send an entity that has no "
            "Sendable component");
        return nullptr;
    }
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::EntitySerializerManager::CreateEntityFromInitialMessage(
    GameWorld* world, Lock &worldlock, ObjectID &returnid, sf::Packet &packet)
{
    // The ID is the first thing in the packet //
    ObjectID id;

    packet >> id;

    returnid = id;
    
    // Get the type from the packet //
    int32_t packettype;

    packet >> packettype;

    if(!packet){

        Logger::Get()->Warning("EntitySerializerManager: CreateEntityFromInitialMessage: "
            "packet didn't have a proper int32_t type, or entity ID");
        return false;
    }
    
    GUARD_LOCK();

    for(size_t i = 0; i < Serializers.size(); i++){

        if(Serializers[i]->DeserializeWholeEntityFromPacket(world, worldlock, id,
                packettype, packet))
        {

            // The correct serializer was at least attempted //
            return true;
        }
    }

    // No serializer was able to do anything //
    return false;
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::EntitySerializerManager::ApplyUpdateMessage(GameWorld* world,
    Lock &worldlock, ObjectID object, sf::Packet &packet, int ticknumber, int referencetick)
{
    // The first thing has to be the type //
    int32_t objtype;

    packet >> objtype;

    if(!packet)
        return false;

    if(!object)
        return false;
    
    GUARD_LOCK();
    
    for(size_t i = 0; i < Serializers.size(); i++){

        if(Serializers[i]->CanSerializeType(objtype)){

            if(Serializers[i]->ApplyUpdateFromPacket(world, worldlock, object, ticknumber,
                    referencetick, packet))
            {

                return true;
            }

            // This might be unnecessary and harmful,
            // as another serializer might be able to do something
            return false;
        }
    }

    // No serializer was able to do anything //
    return false;
}








