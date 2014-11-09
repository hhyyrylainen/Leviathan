// ------------------------------------ //
#ifndef LEVIATHAN_BASESENDABLEENTITY
#include "BaseSendableEntity.h"
#endif
#include "Entities/Objects/Brush.h"
#include "Entities/Objects/Prop.h"
#include "Entities/Objects/TrackEntityController.h"
using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT Leviathan::BaseSendableEntity::BaseSendableEntity(BASESENDABLE_ACTUAL_TYPE type) : SerializeType(type){


}

DLLEXPORT Leviathan::BaseSendableEntity::~BaseSendableEntity(){

}
// ------------------------------------ //
DLLEXPORT void Leviathan::BaseSendableEntity::SerializeToPacket(sf::Packet &packet){

    packet << static_cast<int32_t>(SerializeType);

    _SaveOwnDataToPacket(packet);
}
// ------------------------------------ //
DLLEXPORT unique_ptr<BaseSendableEntity> Leviathan::BaseSendableEntity::UnSerializeFromPacket(sf::Packet &packet,
    GameWorld* world, int id)
{

    int32_t packetstype;
    if(!(packet >> packetstype)){

        return nullptr;
    }

    BASESENDABLE_ACTUAL_TYPE matchtype = static_cast<BASESENDABLE_ACTUAL_TYPE>(packetstype);

    switch(matchtype){
        case BASESENDABLE_ACTUAL_TYPE_BRUSH:
        {
            // This is hopefully written by the Brush _SaveOwnData
            bool hidden;

            if(!(packet >> hidden)){

                return nullptr;
            }
            
            // Create a brush and apply the packet to it //
            unique_ptr<Entity::Brush> tmpobj(new Entity::Brush(hidden, world, id));

            if(!tmpobj->_LoadOwnDataFromPacket(packet)){

                Logger::Get()->Warning("BaseSendableEntity: failed to Init Brush from network packet");
                return nullptr;
            }
            
            return move(unique_ptr<BaseSendableEntity>(dynamic_cast<BaseSendableEntity*>(tmpobj.release())));
        }
        case BASESENDABLE_ACTUAL_TYPE_PROP:
        {
            // This is hopefully written by the Prop _SaveOwnData
            bool hidden;

            if(!(packet >> hidden)){

                return nullptr;
            }
            
            // Create a brush and apply the packet to it //
            unique_ptr<Entity::Prop> tmpobj(new Entity::Prop(hidden, world, id));

            if(!tmpobj->_LoadOwnDataFromPacket(packet)){

                Logger::Get()->Warning("BaseSendableEntity: failed to Init Prop from network packet");
                return nullptr;
            }

            return move(unique_ptr<BaseSendableEntity>(dynamic_cast<BaseSendableEntity*>(tmpobj.release())));
        }
        case BASESENDABLE_ACTUAL_TYPE_TRACKENTITYCONTROLLER:
        {
            unique_ptr<Entity::TrackEntityController> tmpobj(new Entity::TrackEntityController(id, world));

            if(!tmpobj->_LoadOwnDataFromPacket(packet)){

                Logger::Get()->Warning("BaseSendableEntity: failed to Init TrackEntityController from network packet");
                return nullptr;
            }

            return move(unique_ptr<BaseSendableEntity>(dynamic_cast<BaseSendableEntity*>(tmpobj.release())));
        }
        default:
            // Unknown type
            return nullptr;
    }
}
// ------------------------------------ //










