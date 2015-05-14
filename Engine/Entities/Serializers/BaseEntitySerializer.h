#pragma once
// ------------------------------------ //
#include "Define.h"
// ------------------------------------ //
#include "../../Common/SFMLPackets.h"
#include "../Components.h"

namespace Leviathan{

    //! \brief The possible inbuilt types of BaseEntitySerializer
    enum ENTITYSERIALIZEDTYPE{
        //! Type of object that can be serialized with SendableEntitySerializer
        ENTITYSERIALIZEDTYPE_SENDABLE_ENTITY,
        
        //! This has to be the last value, use this to specify custom types
        ENTITYSERIALIZEDTYPE_LAST
    };
    
    //! \brief Base class for all entity serializer classes
    //! \note All possible types should be defined in this file here
	class BaseEntitySerializer{
	public:
        //! Type that should be large enough to hold everything in ENTITYSERIALIZEDTYPE
        //! and all custom types.
        typedef int TypeIDSize;

        //! \brief Creates a serializer which is guaranteed to be able to serialize the type
		DLLEXPORT BaseEntitySerializer(ENTITYSERIALIZEDTYPE type);
        DLLEXPORT virtual ~BaseEntitySerializer();

        //! Returns true when this serializer can work with the specified type
        DLLEXPORT bool CanSerializeType(TypeIDSize typetocheck) const;

        //! \brief Returns true when the object passed can be dynamically casted to the right type
        DLLEXPORT virtual bool IsObjectTypeCorrect(Sendable &object) const = 0;
        
        //! \brief Serializes an entity entirely into a packet
        //! \note This will also link the entity to the connection so that it will automatically send updates
        //! \param connectionptr Pointer to the connection to be used (doesn't have to be verified to be valid)
        //! \return Returns true when the type of object is correct. Which should be when IsObjectTypeCorrect returns
        //! true
        //! \note The caller will have added the ID to the packet so that needs to be skipped (or rather should be)
        //! But the Type variable should be included by this object (as an int32_t)
        DLLEXPORT virtual bool CreatePacketForConnection(GameWorld* world, Lock &worldlock,
            ObjectID id, sf::Packet &packet, ConnectionInfo* connectionptr) = 0;


        //! \brief Deserializes a whole object from a packet if the Type in the packet is the same as Type of this
        //! \note This should do the exact opposite of CreatePacketForConnection
        //! \return True when the type of packet is correct even if the data is invalid
        //! \todo Allow reporting invalid data
        //! \param serializetype The type that was included in the packet by a CreatePacketForConnection
        //! param world The world into which the object is created. Has to be locked before this call
        DLLEXPORT virtual bool DeserializeWholeEntityFromPacket(GameWorld* world, Lock &worldlock, 
            ObjectID id, int32_t serializetype, sf::Packet &packet, int objectid) = 0;


        //! \brief Deserializes and applies an update from a packet
        //! \note The targetobject is found by the caller based on the ID that it inserts
        //! before CreatePacketForConnection
        //! \return True when the type of packet is correct even if the data is invalid
        DLLEXPORT virtual bool ApplyUpdateFromPacket(GameWorld* world, Lock &worldlock,
            ObjectID targetobject, int ticknumber, int referencetick, sf::Packet &packet) = 0;

        
    protected:
        
        //! The type this object can serialize
        //! This is actually of type ENTITYSERIALIZEDTYPE
        const TypeIDSize Type;

        // Disallow copy and assign //
        BaseEntitySerializer(const BaseEntitySerializer &other) = delete;
        BaseEntitySerializer& operator =(const BaseEntitySerializer &other) = delete;
	};
    
}

