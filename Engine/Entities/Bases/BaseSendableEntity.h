#pragma once
// ------------------------------------ //
#include "Define.h"
// ------------------------------------ //
// ---- includes ---- //
#include "BaseObject.h"
#include "../Serializers/BaseEntitySerializer.h"
#include "Common/SFMLPackets.h"
#include "boost/circular_buffer.hpp"

#define BASESENDABLE_STORED_RECEIVED_STATES 4
#define BASESENDABLE_STORED_CLIENT_INTERPOLATIONS 3

#define SENDABLE_RESIMULATE_THRESSHOLD 0.01f


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
        DLLEXPORT ObjectDeltaStateData(int tick);
        DLLEXPORT virtual ~ObjectDeltaStateData();

        //! \brief Adds update data to a packet
        //! \param olderstate The state against which this is compared. Or NULL if a full update is wanted
        DLLEXPORT virtual void CreateUpdatePacket(ObjectDeltaStateData* olderstate, sf::Packet &packet) = 0;

        //! \brief Copies data to missing values in this state from another state
        //! \return True if all missing values have been filled
        DLLEXPORT virtual bool FillMissingData(ObjectDeltaStateData &otherstate) = 0;

        //! \brief The tick this delta state matches
        const int Tick;
        

        ObjectDeltaStateData(const ObjectDeltaStateData &other) = delete;
        void operator=(const ObjectDeltaStateData &other) = delete;
    };

    //! \brief Storage class for ObjectDeltaStateData
    //!
    //! Contains directly the tick number in ObjectDeltaStateData to (hopefully) allow better cache coherence
    //! and improve frame times since the ticks are searched for each frame
    class StoredState{
    public:
        StoredState(std::shared_ptr<ObjectDeltaStateData> data);
        StoredState(StoredState&& other);

        StoredState& operator=(StoredState&& other);
        
        StoredState(const StoredState &other) = delete;
        StoredState& operator=(const StoredState &other) = delete;
        
        std::shared_ptr<ObjectDeltaStateData> DeltaData;
        int Tick;
    };
    
    //! \brief Contains data about a connection and whether the object has changed since last update
    class SendableObjectConnectionUpdate{
        friend BaseSendableEntity;
    public:

        //! \brief Creates an connection holder and creates an initial state
        //! \param curtick The current world tick, used to 
        DLLEXPORT SendableObjectConnectionUpdate(BaseSendableEntity* getstate, Lock &getstatelock,
            ConnectionInfo* connection, int curtick);

        //! \brief Used to update LastConfirmedData
        DLLEXPORT void SucceedOrFailCallback(int ticknumber,
            std::shared_ptr<ObjectDeltaStateData> state, bool succeeded, SentNetworkThing &us);
        
    protected:

        void operator =(SendableObjectConnectionUpdate &other) = delete;
        SendableObjectConnectionUpdate(const SendableObjectConnectionUpdate &other) = delete;
        
        // ------------------------------------ //
        
        //! The connection to which the updates are sent if it is still open
        ConnectionInfo* CorrespondingConnection;

        //! Data used to build a delta update packet
        //! \note This is set to be the last known successfully sent state to avoid having to
        //! resend intermediate steps
        std::shared_ptr<ObjectDeltaStateData> LastConfirmedData;

        //! The tick number of the confirmed state
        //! If a state is confirmed as received that has number higher than this LastConfirmedData will
        //! be replaced.
        int LastConfirmedTickNumber;

        //! Mutex for callback function
        Mutex CallbackMutex;
    };
    
    //! \brief Inherited by objects that can be serialized using the SendableEntitySerializer
	class BaseSendableEntity : public virtual BaseObject{
        friend GameWorld;
	public:
        //! \brief Sets the type identified with this object
        DLLEXPORT BaseSendableEntity(BASESENDABLE_ACTUAL_TYPE type);
        DLLEXPORT virtual ~BaseSendableEntity();
        
        //! \brief Serializes this object to a packet
        //!
        //! Calls the _SaveOwnDataToPacket method after adding SerializeType to the packet
        DLLEXPORT void SerializeToPacket(Lock &guard, sf::Packet &packet);

        //! \brief Used to unserialize objects from packets
        //! \return Returns An object that is derived from BaseSendableEntity on success otherwise nullptr is
        //! returned
        //! \param world The world to which the object is created. This has to be locked before this
        //! call
        DLLEXPORT static std::unique_ptr<BaseSendableEntity> UnSerializeFromPacket(
            sf::Packet &packet, GameWorld* world, int id);

        //! \brief Loads an update from a packet
        DLLEXPORT virtual bool LoadUpdateFromPacket(sf::Packet &packet, int ticknumber,
            int referencetick);

        //! \brief Capture current object state
        //! \param tick The tick value to store in the state, usually world's current tick
        //! \todo Allow this to be cached to improve performance
        DLLEXPORT virtual std::shared_ptr<ObjectDeltaStateData> CaptureState(Lock &guard,
            int tick) = 0;

        //! \brief Subclasses initialize their state object of choice from a packet
        DLLEXPORT virtual std::shared_ptr<ObjectDeltaStateData> CreateStateFromPacket(int tick,
            sf::Packet &packet) const = 0;
        
        //! \brief Tells this entity send updates to all receivers
        //!
        //! An update will be created for each connected ConnectionInfo and then sent.
        //! This will be periodically called by the GameWorld but after, for example, setting the position
        //! this can be called to get clients to update their positions faster
        DLLEXPORT void SendUpdatesToAllClients(int ticknumber);
        
        //! \brief Retrieves states near tick for interpolation
        //! \param progress The progress from the EVENT_TYPE_CLIENT_INTERPOLATION which gets adjusted if the states
        //! aren't 1 tick apart
        //! \exception InvalidState if no states are available
        //! \note use InvalidState to know if queue is empty and InvalidArgument to know if our
        //! tick counter is not in the sweetspot behind the server
        DLLEXPORT void GetServerSentStates(std::shared_ptr<ObjectDeltaStateData> &first,
            std::shared_ptr<ObjectDeltaStateData> &second, int tick, float &progress) const;

        //! \brief Adds a new connection to known receivers
        DLLEXPORT virtual void AddConnectionToReceivers(Lock &guard, ConnectionInfo* receiver);

        //! \brief Returns the sendable type of the object
        DLLEXPORT BASESENDABLE_ACTUAL_TYPE GetSendableType() const;
        
    protected:

        //! \brief Function which is used by subclasses to load their data from packets
        //!
        //! This is called by BaseSendableEntity from UnSerializeFromPacket
        //! This should do the opposite of SerializeToPacket
        //! Note this function should also call Init on the object to make it usable
        //! \note This doesn't require locking as the object cannot be used while this is called
        virtual bool _LoadOwnDataFromPacket(Lock &guard, sf::Packet &packet) = 0;

        //! \brief Child classers use this to serialize their own data to a packet
        //! \note The class implementing this might want to lock while this is being called to avoid properties changing
        virtual void _SaveOwnDataToPacket(Lock &guard, sf::Packet &packet) = 0;

        //! \brief Called internally when data is updated
        //! \note This object has to be locked before this call
        void _MarkDataUpdated(Lock &guard);

        //! \brief Called by entities which want created constraints replicated on clients
        void _SendNewConstraint(BaseConstraintable* us, BaseConstraintable* other, Entity::BaseConstraint* constraint);

        //! \brief Called when a new state has been added notifies the implementation to start listening for
        //! interpolation events
        //! \note This will only be called after 2 states have been received and interpolation
        //! can actually begin
        virtual void _OnNewStateReceived() = 0;
        
        // ------------------------------------ //
        
        //! The serialized type of this object. This is used by this class to create right type of object
        //! from a packet
        BASESENDABLE_ACTUAL_TYPE SerializeType;


        //! Object-wide flag denoting that one or more UpdateReceivers could want an update
        //! \note On the client side this controls whehter an state capture notification actually captures a new state
        bool IsAnyDataUpdated;


        //! Clientside buffer of past states
        //! Use depends on NETWORK_USE_SNAPSHOTS if it is defined this will contain states received from the server
        //! otherwise these are locally captured states
        boost::circular_buffer<StoredState> ClientStateBuffer;
        
    private:
        
        //! List of receivers that will be updated whenever state changes and GameWorld decides it being
        //! the time to send updates
        std::vector<std::shared_ptr<SendableObjectConnectionUpdate>> UpdateReceivers;
    };

}

