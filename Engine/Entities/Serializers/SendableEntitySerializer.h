#pragma once
#ifndef LEVIATHAN_SENDABLEENTITYSERIALIZER
#define LEVIATHAN_SENDABLEENTITYSERIALIZER
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "BaseEntitySerializer.h"

namespace Leviathan{

    //! \brief Serializer for entities that are defined by default (all builtin entities)
    //!
    //! This will be able to serialize the type ENTITYSERIALIZEDTYPE_SENDABLE_ENTITY
	class SendableEntitySerializer : public BaseEntitySerializer{
	public:

        DLLEXPORT SendableEntitySerializer();
        DLLEXPORT ~SendableEntitySerializer();


        //! \copydoc BaseEntitySerializer::CreatePacketForConnection
        DLLEXPORT virtual bool CreatePacketForConnection(BaseObject* object, sf::Packet &packet,
            ConnectionInfo* connectionptr) override;

        //! \copydoc BaseEntitySerializer::DeserializeWholeEntityFromPacket
        DLLEXPORT virtual bool DeserializeWholeEntityFromPacket(BaseObject** returnobj, sf::Packet &packet) override;

        //! \copydoc BaseEntitySerializer::ApplyUpdateFromPacket
        DLLEXPORT virtual bool ApplyUpdateFromPacket(BaseObject* targetobject, sf::Packet &packet) override;


        //! \copydoc BaseEntitySerializer::IsObjectTypeCorrect
        DLLEXPORT virtual bool IsObjectTypeCorrect(BaseObject* object) const override;
        
    protected:

        

	private:
        
		SendableEntitySerializer(const SendableEntitySerializer &other) = delete;
        SendableEntitySerializer& operator =(const SendableEntitySerializer &other) = delete;
	};

}
#endif
