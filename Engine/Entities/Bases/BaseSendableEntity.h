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
        BASESENDABLE_ACTUAL_TYPE_PROP,

        //! The type is Entity::TrackEntityController
        BASESENDABLE_ACTUAL_TYPE_TRACKENTITYCONTROLLER
    };

    //! \brief Base class for entity state data
    class ObjectDeltaStateData{
    public:
        DLLEXPORT virtual ~ObjectDeltaStateData();

        //! \brief Adds update data to a packet
        //! \param olderstate The state against which this is compared. Or NULL if a full update is wanted
        DLLEXPORT virtual void CreateUpdatePacket(ObjectDeltaStateData* olderstate, sf::Packet &packet) = 0;
    };
    
    //! \brief Contains data about a connection and whether the object has changed since last update
    class SendableObjectConnectionUpdate{
        friend BaseSendableEntity;
    public:

        //! \brief Creates an connection holder and creates an initial state
        DLLEXPORT SendableObjectConnectionUpdated(BaseSendableEntity* getstate, ConnectionInfo* connection);


    protected:

        void operator =(SendableObjectConnectionUpdated &other) = delete;
        SendableObjectConnectionUpdated(const SendableObjectConnectionUpdated &other) = delete;
        
        // ------------------------------------ //
        
        //! The connection to which the updates are sent if it is still open
        ConnectionInfo* CorrespondingConnection;

        //! Set to true when the object changes state (moves, changes scale)
        bool DataUpdatedAfterSending;

        //! Data used to build a delta update packet
        //! \note This is set to be the last known successfully sent state to avoid having to
        //! resend intermediate steps
        shared_ptr<ObjectDeltaStateData> LastConfirmedData;
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
        DLLEXPORT static unique_ptr<BaseSendableEntity> UnSerializeFromPacket(sf::Packet &packet, GameWorld* world,
            int id);

        //! \brief Loads an update from a packet
        DLLEXPORT virtual bool LoadUpdateFromPacket(sf::Packet &packet) = 0;

        //! \brief Capture current object state
        //! \todo Allow this to be cached to improve performance
        DLLEXPORT virtual shared_ptr<ObjectDeltaStateData> CaptureState() = 0;
        
        //! \brief Tells this entity send updates to all receivers
        //!
        //! An update will be created for each connected ConnectionInfo and then sent.
        //! This will be periodically called by the GameWorld but after, for example, setting the position
        //! this can be called to get clients to update their positions faster
        DLLEXPORT virtual void SendUpdatesToAllClients();

        //! \brief Adds a new connection to known receivers
        DLLEXPORT virtual void AddConnectionToReceivers(ConnectionInfo* receiver);

        //! \brief Returns the sendable type of the object
        DLLEXPORT BASESENDABLE_ACTUAL_TYPE GetSendableType() const;
        
    protected:
        
        //! \brief Function which is used by subclasses to load their data from packets
        //!
        //! This is called by BaseSendableEntity from UnSerializeFromPacket
        //! This should do the opposite of SerializeToPacket
        //! Note this function should also call Init on the object to make it usable
        //! \note This doesn't require locking as the object cannot be used while this is called
        virtual bool _LoadOwnDataFromPacket(sf::Packet &packet) = 0;

        //! \brief Child classers use this to serialize their own data to a packet
        //! \note The class implementing this might want to lock while this is being called to avoid properties changing
        virtual void _SaveOwnDataToPacket(sf::Packet &packet) = 0;
        
        // ------------------------------------ //
        
        //! The serialized type of this object. This is used by this class to create right type of object
        //! from a packet
        BASESENDABLE_ACTUAL_TYPE SerializeType;


        //! List of receivers that will be updated whenever state changes and GameWorld decides it being
        //! the time to send updates
        std::vector<shared_ptr<SendableObjectConnectionUpdated>> UpdateReceivers;

        //! Object-wide flag denoting that one or more UpdateReceivers could want an update
        bool IsAnyDataUpdated;
    };

}
#endif
