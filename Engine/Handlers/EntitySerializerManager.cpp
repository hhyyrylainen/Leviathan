// ------------------------------------ //
#ifndef LEVIATHAN_ENTITYSERIALIZERMANAGER
#include "EntitySerializerManager.h"
#endif
using namespace Leviathan;
#include "Entities/Serializers/BaseEntitySerializer.h"
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

    GUARD_LOCK_THIS_OBJECT();
    Staticinstance = NULL;

    // Delete the registered serializers //
    SAFE_DELETE_VECTOR(Serializers);
}
// ------------------------------------ //
DLLEXPORT void Leviathan::EntitySerializerManager::AddSerializer(BaseEntitySerializer* serializer){
    if(!serializer)
        return;
    
    GUARD_LOCK_THIS_OBJECT();

    Serializers.push_back(serializer);
}
// ------------------------------------ //
DLLEXPORT unique_ptr<sf::Packet> Leviathan::EntitySerializerManager::CreateInitialEntityMessageFor(BaseObject* object,
    ConnectionInfo* forwho)
{
    GUARD_LOCK_THIS_OBJECT();

    // Setup a packet //
    unique_ptr<sf::Packet> packet(new sf::Packet);

    if(!packet || !object)
        return nullptr;
    
    (*packet) << object->GetID();
    
    // Find a serializer that can finish it //
    for(size_t i = 0; i < Serializers.size(); i++){

        if(Serializers[i]->CreatePacketForConnection(object, *packet, forwho)){

            return move(packet);
        }
    }


    // No serializer was able to do anything //
    return nullptr;
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::EntitySerializerManager::CreateEntityFromInitialMessage(BaseObject** returnobj,
    sf::Packet &packet, GameWorld* world)
{
    GUARD_LOCK_THIS_OBJECT();

    // The ID is the first thing in the packet //
    int32_t id;

    packet >> id;
    
    // Get the type from the packet //
    int32_t packettype;

    packet >> packettype;

    if(!packet){

        Logger::Get()->Warning(L"EntitySerializerManager: CreateEntityFromInitialMessage: packet didn't have a proper"
            L" int32_t type, or entity ID");
        return false;
    }
    

    for(size_t i = 0; i < Serializers.size(); i++){

        if(Serializers[i]->DeserializeWholeEntityFromPacket(returnobj, packettype, packet, id, world)){

            // The correct serializer was at least attempted //
            return true;
        }
    }
    

    // No serializer was able to do anything //
    return false;
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::EntitySerializerManager::ApplyUpdateMessage(sf::Packet &packet,
    boost::function<BaseObject*(int)> objectget)
{
    GUARD_LOCK_THIS_OBJECT();
    DEBUG_BREAK;


    // No serializer was able to do anything //
    return false;
}



