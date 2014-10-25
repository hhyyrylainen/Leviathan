#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_BASESENDABLEENTITY
#include "BaseSendableEntity.h"
#endif
#include "Entities/Objects/Brush.h"
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
    GameWorld* world)
{

    int32_t packetstype;
    if(!(packet >> packetstype)){

        return nullptr;
    }

    BASESENDABLE_ACTUAL_TYPE matchtype = static_cast<BASESENDABLE_ACTUAL_TYPE>(packetstype);

    switch(matchtype){
        case BASESENDABLE_ACTUAL_TYPE_BRUSH:
        {
            // Create a brush and apply the packet to it //
            unique_ptr<Entity::Brush> tmpbrush(new Entity::Brush(false, world));

            DEBUG_BREAK;
            
            return move(unique_ptr<BaseSendableEntity>(dynamic_cast<BaseSendableEntity*>(tmpbrush.release())));
        }
        break;
        default:
            // Unknown type
            return nullptr;
    }
}
// ------------------------------------ //

