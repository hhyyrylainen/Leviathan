#pragma once

//! \file This file contains common components for entities

// ------------------------------------ //
#include "Include.h"
#include "../Common/Types.h"
#include "../Common/SFMLPackets.h"

#include "boost/circular_buffer.hpp"
#include "Serializers/BaseEntitySerializer.h"
#include <functional>
#include "../Newton/PhysicalWorld.h"

#include "Component.h"
#include "CommonStateObjects.h"


namespace Leviathan{

    //! brief Class containing residue static helper functions
    class ComponentHelpers{

        ComponentHelpers() = delete;

    };

    //! \brief Entity has position and direction it is looking at
    //! \note Any possible locking needs to be handled by the caller
	class Position : public Component{
	public:
        
        //! \brief Can hold all data used by Position
        struct PositionData{

            Float3 _Position;
            Float4 _Orientation;
        };

    public:

        DLLEXPORT Position();

        //! \brief Initializes at specific position
        DLLEXPORT bool Init(const Float3 pos, const Float4 rot);

        //! \brief Initializes at 0, 0, 0
        DLLEXPORT bool Init();

        //! \brief Sets _Position and _Orientation to be the same as in the data
        DLLEXPORT void ApplyPositionData(const PositionData &data);

        //! \brief Adds data members to packet
        DLLEXPORT void AddDataToPacket(sf::Packet &packet) const;

        //! \brief Loads data members from packet
        //! \exception InvalidArgument When the packet format is invalid
        DLLEXPORT void ApplyDataFromPacket(sf::Packet &packet);
        
        //! \brief Loads member data to data
        //! \see ApplyPositionData
        DLLEXPORT static void LoadDataFromPacket(sf::Packet &packet, PositionData &data);

        //! \brief Interpolates the member variables between from and to based on progress
        DLLEXPORT void Interpolate(PositionDeltaState &from, PositionDeltaState &to,
            float progress);
        
        

        Float3 _Position;
        Float4 _Orientation;
	};

    //! \brief Entity has an Ogre scene node
    class RenderNode : public Component{
    public:

        DLLEXPORT RenderNode();

        //! \brief Initializes without any Node
        DLLEXPORT bool Init();

        //! \brief Gracefully releases while world is still valid
        DLLEXPORT void Release(Ogre::SceneManager* worldsscene);
        
        
        Ogre::SceneNode* Node;
    };

    //! \brief Types for Sendable
    //!
    //! Determines which components should be searched for and then sent
    enum SENDABLE_TYPE{

        //! Invalid type, if this is initialized to null errors can be caught
        SENDABLE_TYPE_INVALID = 0,

        //! The sendable is a Brush
        //! Components: Position, RenderNode, BoxGeometry, Physics, Constraintable
        SENDABLE_TYPE_BRUSH,

        //! The sendable is a Prop
        //! Components: Position, RenderNode, Model, Physics, Constraintable
        SENDABLE_TYPE_PROP,

        //! The sendable is a TrackController
        //! Components: PhysicsListener, PositionMarkerOwner, Parent
        SENDABLE_TYPE_TRACKCONTROLLER
    };

#define BASESENDABLE_STORED_RECEIVED_STATES 4
#define BASESENDABLE_STORED_CLIENT_INTERPOLATIONS 3
    

    //! \brief Entity is sendable to clients
    //! \note This will only be in the entity on the server
    class Sendable : public Component{

        struct ActiveConnection{
            
            ConnectionInfo* CorrespondingConnection;

            //! Data used to build a delta update packet
            //! \note This is set to be the last known successfully sent state to avoid having to
            //! resend intermediate steps
            std::shared_ptr<ObjectDeltaStateData> LastConfirmedData;

            //! The tick number of the confirmed state
            //! If a state is confirmed as received that has number higher than this
            // LastConfirmedData will be replaced.
            int LastConfirmedTickNumber;

            //! Mutex for callback function
            Mutex CallbackMutex;
        };
        
    public:
        
        DLLEXPORT Sendable();

        //! \brief Inits sendable with specified type
        DLLEXPORT bool Init(SENDABLE_TYPE type);

        //! \brief Adds SendableHandleType to packet
        DLLEXPORT void AddTypeToPacket(sf::Packet &packet) const;

        DLLEXPORT static SENDABLE_TYPE LoadTypeFromPacket(sf::Packet &packet);
        
        //! Type used to find the required components for sending
        SENDABLE_TYPE SendableHandleType;

        //! Clients we have already sent a state to
        std::vector<std::unique_ptr<ActiveConnection>> UpdateReceivers;
    };

    //! \brief Entity is received from a server
    //!
    //! Sendable counterpart on the client
    class Received : public Component{
        //! \brief Storage class for ObjectDeltaStateData
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
    public:

        //! Clientside buffer of past states
        boost::circular_buffer<StoredState> ClientStateBuffer;
    };


    //! \brief Entity has a box for geometry/model, possibly also physics
    class BoxGeometry : public Component{
    public:
        

    };


    //! \brief Entity has a physical component
    //! \pre Entity has Position component
    //! \todo Global Newton lock
    class Physics : public Component{
    public:

        //! \brief Holder for information regarding a single force
        class ApplyForceInfo{
        public:
            //! \note Pass NULL for name if not used, avoid passing empty strings
            //! \param name The name to assign. This will be deleted by a std::unique_ptr
            DLLEXPORT ApplyForceInfo(bool addmass,
                std::function<Float3(ApplyForceInfo* instance, Physics &object,
                    Lock &objectlock)> getforce,
                std::string* name = NULL);
        
            DLLEXPORT ApplyForceInfo(ApplyForceInfo &other);
            DLLEXPORT ApplyForceInfo(ApplyForceInfo &&other);
            DLLEXPORT ~ApplyForceInfo();

            DLLEXPORT ApplyForceInfo& operator =(const ApplyForceInfo &other);

            //! Set a name when you don't want other non-named forces to override this
            std::unique_ptr<std::string> OptionalName;
        
            //! Whether to multiply the force by mass, makes acceleration constant with
            //! different masses
            bool MultiplyByMass;
        
            //! The callback which returns the force
            //! \todo Allow deleting this force from the callback
            std::function<Float3(ApplyForceInfo* instance, Physics &object,
                Lock &objectlock)> Callback;
        };

        struct BasePhysicsData{

            Float3 Velocity;
            Float3 Torque;
        };
        
    public:
        

        
		DLLEXPORT void GiveImpulse(const Float3 &deltaspeed, const Float3 &point = Float3(0));

		//! \brief Adds an apply force
        //! \note Overwrites old forces with the same name
        //! \param pointertohandle Pointer to the force which will be deleted by this
		DLLEXPORT void ApplyForce(ApplyForceInfo* pointertohandle);

		//! \brief Removes an existing ApplyForce
        //! \param name name of force to delete, pass empty std::string to delete the
        //! default named force
		DLLEXPORT bool RemoveApplyForce(const std::string &name);

        //! \brief Sets absolute velocity of the object
        DLLEXPORT void SetVelocity(const Float3 &velocities);

        //! \brief Gets the absolute velocity
        DLLEXPORT Float3 GetBodyVelocity(Lock &guard);

        //! \brief Sets the torque of the body
        //! \see GetBodyTorque
        DLLEXPORT void SetTorque(const Float3 &torque);

        //! \brief Gets the torque of the body (rotational velocity)
        DLLEXPORT Float3 GetTorque();

        //! \brief Sets the physical material ID of this object
        //! \note You have to fetch the ID from the world's corresponding PhysicalMaterialManager
		DLLEXPORT void SetPhysicalMaterialID(Lock &guard, int ID);

        //! \brief Sets the linear dampening which slows down the object
        //! \param factor The factor to set. Must be between 0.f and 1.f. Default is 0.1f
        //! \note This can be used to set the viscosity of the substance the object is in
        //! for example to mimic drag in water (this needs verification...)
        //!
        //! More on this in the Newton wiki here:
        //! http://newtondynamics.com/wiki/index.php5?title=NewtonBodySetLinearDamping
        DLLEXPORT void SetLinearDampening(float factor = 0.1f);

        //! \brief Applies physical state from holder object
        DLLEXPORT void ApplyPhysicalState(const BasePhysicsData &data);

    protected:

        // default physics callbacks that are fine in most cases //
		// Don't forget to pass the user data as BaseObject if using these //
		static void ApplyForceAndTorgueEvent(const NewtonBody* const body, dFloat timestep,
            int threadIndex);
        
		static void DestroyBodyCallback(const NewtonBody* body);

        static void PropPhysicsMovedEvent(const NewtonBody* const body, const dFloat* const matrix,
            int threadIndex);

        
        //! \brief Adds all applied forces together
		Float3 _GatherApplyForces(Lock &guard, const float &mass);
        

    public:

        //! \brief Destroys the physical body
        DLLEXPORT void Release(NewtonWorld* world);

        //! \brief Moves the physical body to the specified position
        DLLEXPORT void JumpTo(Position &target);

        
        
		NewtonCollision* Collision;
		NewtonBody* Body;

        //! The set physical material
        //! If none is set this defaults to -1
        //! The default material ID from GetDefaultPhysicalMaterialID might be applied
        int AppliedPhysicalMaterial;

		bool Immovable;
		bool ApplyGravity;

		std::list<std::shared_ptr<ApplyForceInfo>> ApplyForceList;
    };

    class Parent : public Component{
    public:
        

    };

    class Parentable : public Component{
    public:

        
    };


    class Constraintable : public Component{
    public:
        

    };

    class PhysicsListener : public Component{


    };

    class Trail : public Component{
    public:

        struct Properties{


        };
        

    public:
        

        Properties CurrentSettings;
    };
    
    class PositionMarkerOwner : public Component{


    };

    
}
