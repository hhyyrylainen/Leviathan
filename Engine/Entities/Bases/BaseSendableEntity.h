#pragma once
#ifndef LEVIATHAN_BASESENDABLEENTITY
#define LEVIATHAN_BASESENDABLEENTITY
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "BaseObject.h"
#include "../Serializers/BaseEntitySerializer.h"
#include "SFML/Network/Packet.hpp"

namespace Leviathan{

    //! \brief The internal object type that the BaseSendableEntity child class is
    enum BASESENDABLE_ACTUAL_TYPE {
        //! Invalid type
        BASESENDABLE_ACTUAL_TYPE_ERROR = 0,

        //! The type is Entity::Brush
        BASESENDABLE_ACTUAL_TYPE_BRUSH,

        //! The type is Entity::Prop
        BASESENDABLE_ACTUAL_TYPE_PROP
    };
    
    //! \brief Inherited by objects that can be serialized using the SendableEntitySerializer
	class BaseSendableEntity : public virtual BaseObject{
	public:
        //! \brief Sets the type identified with this object
        DLLEXPORT BaseSendableEntity(BASESENDABLE_ACTUAL_TYPE type);
        DLLEXPORT virtual ~BaseSendableEntity();
        
        //! \brief Serializes this object to a packet
        //!
        //! Calls the _SaveOwnDataToPacket method after adding SerializeType to the packet
        DLLEXPORT void SerializeToPacket(sf::Packet &packet);

        //! \brief Used to unserialize objects from packets
        //! \return Returns An object that is derived from BaseSendableEntity on success otherwise nullptr is
        //! returned
        //! \param world The world to which the object is created. This has to be locked before this
        //! call
        DLLEXPORT static unique_ptr<BaseSendableEntity> UnSerializeFromPacket(sf::Packet &packet, GameWorld* world);

        //! \brief Serializes an update to a packet for a specific connection
        //! \note This function should include it's own type in the packet to verify that it is the right type
        DLLEXPORT virtual void AddUpdateToPacket(sf::Packet &packet, ConnectionInfo* receiver) = 0;

        //! \brief Loads an update from a packet
        DLLEXPORT virtual bool LoadUpdateFromPacket(sf::Packet &packet) = 0;
        
        

    protected:
        
        //! \brief Function which is used by subclasses to load their data from packets
        //!
        //! This is called by BaseSendableEntity from UnSerializeFromPacket
        //! This should do the opposite of SerializeToPacket
        //! Note this function should also call Init on the object to make it usable
        virtual bool _LoadOwnDataFromPacket(sf::Packet &packet) = 0;

        //! \brief Child classers use this to serialize their own data to a packet
        virtual void _SaveOwnDataToPacket(sf::Packet &packet) = 0;
        
        // ------------------------------------ //
        
        //! The serialized type of this object. This is used by this class to create right type of object
        //! from a packet
        BASESENDABLE_ACTUAL_TYPE SerializeType;


    };

}
#endif
