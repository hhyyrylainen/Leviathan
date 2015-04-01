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
#include "Common/SFMLPackets.h"
#include "boost/thread/mutex.hpp"
#include "boost/circular_buffer.hpp"

#ifndef NETWORK_USE_SNAPSHOTS
#define BASESENDABLE_STORED_CLIENT_STATES 14
#else
#define BASESENDABLE_STORED_RECEIVED_STATES 4
#define BASESENDABLE_STORED_CLIENT_INTERPOLATIONS 3
#endif //NETWORK_USE_SNAPSHOTS

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

        //! \brief The tick this delta state matches
        const int Tick;
        

        ObjectDeltaStateData(const ObjectDeltaStateData &other) = delete;
        void operator=(const ObjectDeltaStateData &other) = delete;
    };

#ifdef NETWORK_USE_SNAPSHOTS
    //! \brief Contains an interpolation for client side entity to perform
    struct ObjectInterpolation{
    public:
        ObjectInterpolation(shared_ptr<ObjectDeltaStateData> first, shared_ptr<ObjectDeltaStateData> second,
            int duration);

        //! Move constructor
        ObjectInterpolation(ObjectInterpolation &&other);

        //! The first state
        //! Must always be valid
        shared_ptr<ObjectDeltaStateData> First;
        
        //! The second state
        //! Must always be valid
        shared_ptr<ObjectDeltaStateData> Second;

        //! The time the change should take in milliseconds
        //! Must be > 0
        int Duration;
    };
    
#endif //NETWORK_USE_SNAPSHOTS

    
    //! \brief Contains data about a connection and whether the object has changed since last update
    class SendableObjectConnectionUpdate{
        friend BaseSendableEntity;
    public:

        //! \brief Creates an connection holder and creates an initial state
        //! \param curtick The current world tick, used to 
        DLLEXPORT SendableObjectConnectionUpdate(BaseSendableEntity* getstate, ConnectionInfo* connection, int curtick);

        //! \brief Used to update LastConfirmedData
        DLLEXPORT void SucceedOrFailCallback(int ticknumber, shared_ptr<ObjectDeltaStateData> state,
            bool succeeded, SentNetworkThing &us);
        
    protected:

        void operator =(SendableObjectConnectionUpdate &other) = delete;
        SendableObjectConnectionUpdate(const SendableObjectConnectionUpdate &other) = delete;
        
        // ------------------------------------ //
        
        //! The connection to which the updates are sent if it is still open
        ConnectionInfo* CorrespondingConnection;

        //! Set to true when the object changes state (moves, changes scale)
        bool DataUpdatedAfterSending;

        //! Data used to build a delta update packet
        //! \note This is set to be the last known successfully sent state to avoid having to
        //! resend intermediate steps
        shared_ptr<ObjectDeltaStateData> LastConfirmedData;

        //! The tick number of the confirmed state
        //! If a state is confirmed as received that has number higher than this LastConfirmedData will
        //! be replaced.
        int LastConfirmedTickNumber;

        //! Mutex for callback function
        boost::mutex CallbackMutex;
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
        DLLEXPORT void SerializeToPacket(sf::Packet &packet);

        //! \brief Used to unserialize objects from packets
        //! \return Returns An object that is derived from BaseSendableEntity on success otherwise nullptr is
        //! returned
        //! \param world The world to which the object is created. This has to be locked before this
        //! call
        DLLEXPORT static unique_ptr<BaseSendableEntity> UnSerializeFromPacket(sf::Packet &packet, GameWorld* world,
            int id);

        //! \brief Loads an update from a packet
        DLLEXPORT virtual bool LoadUpdateFromPacket(sf::Packet &packet, int ticknumber);

        //! \brief Capture current object state
        //! \param tick The tick value to store in the state, usually world's current tick
        //! \todo Allow this to be cached to improve performance
        DLLEXPORT virtual shared_ptr<ObjectDeltaStateData> CaptureState(int tick) = 0;

#ifndef NETWORK_USE_SNAPSHOTS
        //! \brief Verifies that an old server state is consistent with the client state
        //! \param serversold The server's state that we have received
        //! \param ourold Our old state that matches the tick, if any (only exact tick number matches are counted)
        //! \param tick The tick on which the state was captured
        DLLEXPORT virtual void VerifyOldState(ObjectDeltaStateData* serversold, ObjectDeltaStateData* ourold,
            int tick) = 0;
        
#endif //NETWORK_USE_SNAPSHOTS

        //! \brief Subclasses initialize their state object of choice from a packet
        DLLEXPORT virtual shared_ptr<ObjectDeltaStateData> CreateStateFromPacket(int tick,
            sf::Packet &packet) const = 0;
        
        //! \brief Tells this entity send updates to all receivers
        //!
        //! An update will be created for each connected ConnectionInfo and then sent.
        //! This will be periodically called by the GameWorld but after, for example, setting the position
        //! this can be called to get clients to update their positions faster
        DLLEXPORT void SendUpdatesToAllClients(int ticknumber);

#ifndef NETWORK_USE_SNAPSHOTS
        //! \brief Tells this entity to capture its client side state
        //! \note It will only be captured if the object is marked as updated
        //! \warning This may NOT be called on any other application than a client
        //! \see IsAnyDataUpdated
        DLLEXPORT void StoreClientSideState(int ticknumber);

        //! \brief Replaces an old state with a newer one
        //!
        //! This is used on the client when resimulating to replace old invalid states
        DLLEXPORT bool ReplaceOldClientState(int onticktoreplace, shared_ptr<ObjectDeltaStateData> state);

#else

        //! \brief Queues a interpolation for this entity
        //! \param mstime The time the interpolation should take in milliseconds
        DLLEXPORT void QueueInterpolation(shared_ptr<ObjectDeltaStateData> from, shared_ptr<ObjectDeltaStateData> to,
            int mstime);

        //! \brief Retrieves and pops the next interpolation
        //! \brief ExceptionInvalidState when none in queue
        DLLEXPORT ObjectInterpolation GetAndPopNextInterpolation() THROWS;
        
#endif //NETWORK_USE_SNAPSHOTS

        //! \brief Adds a new connection to known receivers
        DLLEXPORT virtual void AddConnectionToReceivers(ConnectionInfo* receiver);

        //! \brief Returns the sendable type of the object
        DLLEXPORT BASESENDABLE_ACTUAL_TYPE GetSendableType() const;
        
    protected:

#ifdef NETWORK_USE_SNAPSHOTS

        //! \brief Report view interpolation status to the input manager
        //! \param tick The tick that will be reached at millisecond time mstime
        static void ReportInterpolationStatusToInput(int tick, int64_t mstime);


        //! \brief Start a new interpolation if current one is finished
        virtual void VerifySendableInterpolation() = 0;

#endif //NETWORK_USE_SNAPSHOTS
        
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

        //! \brief Called internally when data is updated
        //! \note This object has to be locked before this call
        void _MarkDataUpdated(ObjectLock &guard);

        //! \brief Called by entities which want created constraints replicated on clients
        void _SendNewConstraint(BaseConstraintable* us, BaseConstraintable* other, Entity::BaseConstraint* constraint);
        
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
        boost::circular_buffer<shared_ptr<ObjectDeltaStateData>> ClientStateBuffer;

#ifdef NETWORK_USE_SNAPSHOTS

        //! Clientside list of queued interpolation states
        std::deque<ObjectInterpolation> QueuedInterpolationStates;
        

#endif //NETWORK_USE_SNAPSHOTS
        

        //! The tick on which the client state was last checked with the server
        //! any updates older than this will be ignored
        int LastVerifiedTick;

    private:
        
        //! List of receivers that will be updated whenever state changes and GameWorld decides it being
        //! the time to send updates
        std::vector<shared_ptr<SendableObjectConnectionUpdate>> UpdateReceivers;
    };

}
#endif
