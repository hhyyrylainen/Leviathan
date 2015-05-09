// ------------------------------------ //
#include "EntitySerializerManager.h"

#include "Entities/Serializers/BaseEntitySerializer.h"
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
    BaseObject* object, ConnectionInfo* forwho)
{
    GUARD_LOCK();

    // Setup a packet //
    std::unique_ptr<sf::Packet> packet(new sf::Packet);

    if(!packet || !object)
        return nullptr;
    
    (*packet) << object->GetID();
    
    // Find a serializer that can finish it //
    for(size_t i = 0; i < Serializers.size(); i++){

        if(Serializers[i]->CreatePacketForConnection(object, guard, *packet, forwho)){

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
    GUARD_LOCK();

    // The ID is the first thing in the packet //
    int32_t id;

    packet >> id;
    
    // Get the type from the packet //
    int32_t packettype;

    packet >> packettype;

    if(!packet){

        Logger::Get()->Warning("EntitySerializerManager: CreateEntityFromInitialMessage: "
            "packet didn't have a proper int32_t type, or entity ID");
        return false;
    }

    for(size_t i = 0; i < Serializers.size(); i++){

        if(Serializers[i]->DeserializeWholeEntityFromPacket(returnobj, packettype, packet, id,
                world))
        {

            // The correct serializer was at least attempted //
            return true;
        }
    }
    

    // No serializer was able to do anything //
    return false;
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::EntitySerializerManager::ApplyUpdateMessage(sf::Packet &packet, int ticknumber,
    int referencetick, ObjectPtr object)
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

            if(Serializers[i]->ApplyUpdateFromPacket(object.get(), ticknumber, referencetick,
                    packet))
            {

                return true;
            }

            // This might be unnecessary and harmful, as another serializer might be able to do something //
            return false;
        }
    }

    // No serializer was able to do anything //
    return false;
}








