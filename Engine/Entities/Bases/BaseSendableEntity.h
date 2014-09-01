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
#include "SFML/Packet.hpp"

namespace Leviathan{

    //! \brief Inherited by objects that can be serialized using the SendableEntitySerializer
	class BaseSendableEntity : public virtual BaseObject{
	public:
        //! \brief Sets the type identified with this object
		DLLEXPORT BaseSendableEntity(BaseEntitySerializer::TypeIDSize type);
        DLLEXPORT virtual ~BaseSendableEntity();
        
        //! \brief Implemented by subclasses to be serialized
        DLLEXPORT virtual void SerializeToPacket(sf::Packet &packet) = 0;

        //! \brief Used to unserialize objects from packets
        //! \return Returns true on success
        //! \note If this function fails the resulting object state should be considered to be invalid and thus 
        //! this should be released and deleted to avoid issues.
        DLLEXPORT virtual bool UnSerializeFromPacket(sf::Packet &packet) = 0;

    protected:

        //! The serialized type of this object. This is used to identify the unpacker that is used to recover this
        //! object
        BaseEntitySerializer::TypeIDSize SerializeType;
        
    };

}
#endif
