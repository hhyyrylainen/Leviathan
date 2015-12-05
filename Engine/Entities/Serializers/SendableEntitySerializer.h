#pragma once
// ------------------------------------ //
#include "Define.h"
// ------------------------------------ //
#include "BaseEntitySerializer.h"
#include "../Components.h"

namespace Leviathan{

    //! \brief Serializer for entities that are defined by default (all builtin entities)
    //!
    //! This will be able to serialize the type ENTITYSERIALIZEDTYPE_SENDABLE_ENTITY
	class SendableEntitySerializer : public BaseEntitySerializer{
	public:

        DLLEXPORT SendableEntitySerializer();
        DLLEXPORT ~SendableEntitySerializer();


        //! \copydoc BaseEntitySerializer::CreatePacketForConnection
        DLLEXPORT virtual bool CreatePacketForConnection(GameWorld* world, Lock &worldlock,
            ObjectID id, Sendable &sendable, sf::Packet &packet, ConnectionInfo* connectionptr)
            override;

        //! \copydoc BaseEntitySerializer::DeserializeWholeEntityFromPacket
        DLLEXPORT virtual bool DeserializeWholeEntityFromPacket(GameWorld* world, Lock &worldlock, 
            ObjectID id, int32_t serializetype, sf::Packet &packet) override;

        //! \copydoc BaseEntitySerializer::ApplyUpdateFromPacket
        DLLEXPORT virtual bool ApplyUpdateFromPacket(GameWorld* world, Lock &worldlock,
            ObjectID targetobject, int ticknumber, int referencetick, sf::Packet &packet) override;


        //! \copydoc BaseEntitySerializer::IsObjectTypeCorrect
        DLLEXPORT virtual bool IsObjectTypeCorrect(Sendable &object) const override;

    protected:

        DLLEXPORT bool VerifyAndFillReceivedState(Received* received, int ticknumber,
            int referencetick, std::shared_ptr<ObjectDeltaStateData> receivedstate);
	};

}

