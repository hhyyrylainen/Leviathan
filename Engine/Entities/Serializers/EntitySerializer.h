// Leviathan Game Engine
// Copyright (c) 2012-2016 Henri Hyyryläinen
#pragma once
// ------------------------------------ //
#include "Define.h"
// ------------------------------------ //
#include "../../Common/SFMLPackets.h"
#include "Entities/Components.h"
#include "../../Common/ThreadSafe.h"

namespace Leviathan{

//! \brief Base class for all entity serializer classes
//! \note All possible types should be defined in this file here
class EntitySerializer{
public:

    //! \brief Creates a serializer which is guaranteed to be able to serialize the type
    inline EntitySerializer() {
    }

    virtual ~EntitySerializer(){ }

    //! \brief Serializes an entity entirely into a packet
    //! \note This will also link the entity to the connection so that it will
    //! automatically send updates
    //! \param connectionptr Pointer to the connection to be used
    //! (doesn't have to be verified to be valid)
    //! \return Returns true when the type of object is correct.
    //! Which should be when IsObjectTypeCorrect returns true
    //! \note The caller will have added the ID to the packet so that needs to be skipped
    //! (or rather should be) But the Type variable should be included by
    //! this object (as an int32_t)
    DLLEXPORT virtual bool CreatePacketForConnection(GameWorld* world, Lock &worldlock,
        ObjectID id, Sendable &sendable, sf::Packet &packet,
        Connection &connection);


    //! \brief Deserializes a whole object from a packet if the Type in the packet is
    //! the same as Type of this
    //! \note This should do the exact opposite of CreatePacketForConnection
    //! \return True when the type of packet is correct even if the data is invalid
    //! \todo Allow reporting invalid data
    //! \param serializetype The type that was included in the packet by
    //! a CreatePacketForConnection
    //! \param world The world into which the object is created.
    //! Has to be locked before this call
    DLLEXPORT virtual bool DeserializeWholeEntityFromPacket(GameWorld* world,
        Lock &worldlock, ObjectID id, sf::Packet &packet);


    //! \brief Deserializes and applies an update from a packet
    //! \note The targetobject is found by the caller based on the ID that it inserts
    //! before CreatePacketForConnection
    //! \return True when the type of packet is correct even if the data is invalid
    DLLEXPORT virtual bool ApplyUpdateFromPacket(GameWorld* world, Lock &worldlock,
        ObjectID targetobject, int ticknumber, int referencetick, sf::Packet &packet);

        
protected:

    DLLEXPORT bool VerifyAndFillReceivedState(Received* received, int ticknumber,
        int referencetick, std::shared_ptr<ComponentState> receivedstate);
        
    // Disallow copy and assign //
    EntitySerializer(const EntitySerializer &other) = delete;
    EntitySerializer& operator =(const EntitySerializer &other) = delete;
};
    
}

