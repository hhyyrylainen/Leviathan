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
#include "Objects/Constraints.h"


namespace Leviathan{

    
    static const int BASESENDABLE_STORED_RECEIVED_STATES = 4;


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

        //! \brief Creates at specific position
        DLLEXPORT Position(const Float3 pos, const Float4 rot);

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
        DLLEXPORT void Interpolate(const PositionDeltaState &from, const PositionDeltaState &to,
            float progress);
        
        

        Float3 _Position;
        Float4 _Orientation;
	};

    //! \brief Entity has an Ogre scene node
    //! \note By default this is not marked. If you change Hidden set as marked to
    //! update Node state
    class RenderNode : public Component{
    public:

        DLLEXPORT RenderNode();

        //! \brief Initializes without any Node
        DLLEXPORT bool Init();

        //! \brief Gracefully releases while world is still valid
        DLLEXPORT void Release(Ogre::SceneManager* worldsscene);
        
        Ogre::SceneNode* Node;

        //! Sets objects attached to the node to be hidden or visible
        bool Hidden;
    };

    //! \brief Types for Sendable
    //!
    //! Determines which components should be searched for and then sent
    enum SENDABLE_TYPE{

        //! Invalid type, if this is initialized to null errors can be caught
        SENDABLE_TYPE_INVALID = 0,

        //! The sendable is a Brush
        //! Components: Position, RenderNode, BoxGeometry, Physics, Constraintable
        //! Type is PositionDeltaState
        SENDABLE_TYPE_BRUSH,

        //! The sendable is a Prop
        //! Components: Position, RenderNode, Model, Physics, Constraintable
        //! Type is PositionDeltaState
        SENDABLE_TYPE_PROP,

        //! The sendable is a TrackController
        //! Components: PhysicsListener, PositionMarkerOwner, Parent
        //! Type is TrackControllerState
        SENDABLE_TYPE_TRACKCONTROLLER
    };

    //! \brief Entity is sendable to clients
    //! \note This will only be in the entity on the server
    class Sendable : public Component{
    public:
        struct ActiveConnection{

            ActiveConnection(ConnectionInfo* connection);

            DLLEXPORT void OnPacketFinalized(std::shared_ptr<ActiveConnection> object,
                int tick, std::shared_ptr<ObjectDeltaStateData> state, bool succeded,
                SentNetworkThing &packet);
            
            
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
        
        DLLEXPORT Sendable(SENDABLE_TYPE type);

        DLLEXPORT void AddConnectionToReceivers(Lock &guard, ConnectionInfo* connection);

        //! Type used to find the required components for sending
        SENDABLE_TYPE SendableHandleType;

        //! Clients we have already sent a state to
        std::vector<std::shared_ptr<ActiveConnection>> UpdateReceivers;
    };

    //! \brief Entity is received from a server
    //!
    //! Sendable counterpart on the client
    class Received : public Component{
    public:
        //! \brief Storage class for ObjectDeltaStateData
        //! \todo Possibly add move constructors
        class StoredState{
        public:
            StoredState(std::shared_ptr<ObjectDeltaStateData> safedata, void* data,
                SENDABLE_TYPE datatype);
        
            std::shared_ptr<ObjectDeltaStateData> DeltaData;
            int Tick;

            //! This avoids using dynamic_cast
            void* DirectData;

            //! Type of the owning Received used to reinterpret_cast to correct type
            SENDABLE_TYPE OwnersType;
        };
    public:

        DLLEXPORT Received(SENDABLE_TYPE type);

        DLLEXPORT void GetServerSentStates(Lock &guard, StoredState const** first,
            StoredState const** second, int tick, float &progress) const;

        DLLEXPORT inline void GetServerSentStates(StoredState const** first,
            StoredState const** second, int tick, float &progress) const
        {
            GUARD_LOCK();
            GetServerSentStates(guard, first, second, tick, progress);
        }


        //! Clientside buffer of past states
        boost::circular_buffer<StoredState> ClientStateBuffer;

        //! Type used to create correct state objects and construct from packets
        SENDABLE_TYPE SendableHandleType;
    };


    //! \brief Entity has a box for geometry/model, possibly also physics
    class BoxGeometry : public Component{
    public:
        DLLEXPORT BoxGeometry(const Float3 &size, const std::string &material);
        
        //! Size along the axises
        Float3 Sizes;

        //! Rendering surface material name
        std::string Material;

        //! Entity created from a box mesh
        Ogre::Entity* GraphicalObject;
    };

    //! \brief Entity has a model
    class Model : public Component{
    public:
        DLLEXPORT Model(const std::string &file);

        //! \brief Destroys GraphicalObject
        DLLEXPORT void Release(Ogre::SceneManager* scene);
        
        std::string ModelFile;

        //! The entity that has this model's mesh loaded
        Ogre::Entity* GraphicalObject;
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
                std::unique_ptr<std::string> name = nullptr);
        
            DLLEXPORT ApplyForceInfo(const ApplyForceInfo &other);
            DLLEXPORT ApplyForceInfo(ApplyForceInfo &&other);

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

        struct Arguments{

            ObjectID id;
            GameWorld* world;
            Position &updatepos;
            Sendable* updatesendable;
        };
        
    public:
        
        DLLEXPORT Physics(const Arguments &args);

        DLLEXPORT void Release();
        
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
        DLLEXPORT void SetVelocity(Lock &guard, const Float3 &velocities);

        //! \brief Gets the absolute velocity
        DLLEXPORT Float3 GetVelocity(Lock &guard) const;

        DLLEXPORT inline Float3 GetVelocity() const{

            GUARD_LOCK();
            return GetVelocity(guard);
        }

        DLLEXPORT NewtonBody* GetBody() const;

        //! \brief Sets the torque of the body
        //! \see GetBodyTorque
        DLLEXPORT void SetTorque(Lock &guard, const Float3 &torque);

        //! \brief Gets the torque of the body (rotational velocity)
        DLLEXPORT Float3 GetTorque(Lock &guard);

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

        

        // default physics callbacks that are fine in most cases //
		// Don't forget to pass the user data as BaseObject if using these //
		static void ApplyForceAndTorgueEvent(const NewtonBody* const body, dFloat timestep,
            int threadIndex);
        
		static void DestroyBodyCallback(const NewtonBody* body);

        static void PhysicsMovedEvent(const NewtonBody* const body, const dFloat* const matrix,
            int threadIndex);

        
        //! \brief Adds all applied forces together
		Float3 _GatherApplyForces(Lock &guard, const float &mass);
        
        //! \todo Fix this to follow the new requirements (or this will segfault)
        DLLEXPORT void InterpolatePhysicalState(PhysicalDeltaState &first,
            PhysicalDeltaState &second, float progress);

        //! \brief Destroys the physical body
        DLLEXPORT void Release(NewtonWorld* world);

        //! \brief Moves the physical body to the specified position
        DLLEXPORT void JumpTo(Position &target);

        DLLEXPORT bool SetPosition(Lock &guard, const Float3 &pos, const Float4 &orientation);

        
        
		NewtonCollision* Collision;
		NewtonBody* Body;

        //! The set physical material
        //! If none is set this defaults to -1
        //! The default material ID from GetDefaultPhysicalMaterialID might be applied
        int AppliedPhysicalMaterial;

		bool ApplyGravity;

        //! Non-newton access to mass
        float Mass;

		std::list<std::shared_ptr<ApplyForceInfo>> ApplyForceList;

        //! Used to access gravity data
        GameWorld* World;

        //! Physics object requires a position
        Position& _Position;

        //! For access from physics callbacks
        ObjectID ThisEntity;

        // Optional access to other components that can be used for marking when physics object
        // moves
        Sendable* UpdateSendable;
    };

    class Parent : public Component{
    public:

        struct Data{

            std::vector<ObjectID> EntityIDs;
        };
        
    public:
        DLLEXPORT Parent();

        DLLEXPORT Parent(const Data &data, GameWorld* world, Lock &worldlock);

        //! \brief Removes child object without notifying it
        DLLEXPORT void RemoveChildNoNotify(Parentable* which);

        //! \brief Removes all children notifying them
        DLLEXPORT void RemoveChildren();

        DLLEXPORT void AddDataToPacket(sf::Packet &packet) const;

        //! \note The packet needs to be checked that it is still valid after this call
        DLLEXPORT static Data LoadDataFromPacket(sf::Packet &packet);

        //! \brief Does everything necessary to attach a child
        DLLEXPORT void AddChild(ObjectID childid, Parentable &child);

        //! Attached children which can be moved at certain times
        //! \todo Make improvements to component lookup performance through this
        std::vector<std::tuple<ObjectID, Parentable*>> Children;
    };

    class Parentable : public Component{
    public:
        DLLEXPORT Parentable();

        //! \brief Detaches from parent
        //!
        //! Sets AttachedParent to NULL and notifies Parent
        DLLEXPORT void DetachFromParent();

        //! \brief Sets AttachedParent to NULL
        DLLEXPORT void OnParentInvalidate();

        //! \todo Make this work
        Float3 RelativeToParent;

        //! \todo Make this work
        bool ApplyRotation;

        //! Parent this is attached to, or NULL
        Parent* AttachedParent;
    };


    class Constraintable : public Component{
    public:
        //! \param world Used to allow created constraints to access world data (including physics)
        DLLEXPORT Constraintable(ObjectID id, GameWorld* world);

        //! \brief Destroys all constraints attached to this
        DLLEXPORT ~Constraintable();

        //! Creates a constraint between this and another object
        //! \warning DO NOT store the returned value (since that reference isn't counted)
        //! \note Before the constraint is actually finished, you need to
        //! call ->SetParameters() on the returned ptr
        //! and then ->Init() and then let go of the ptr
        //! \note If you do not want to allow constraints where child is NULL you have to
        //! check if child is NULL before calling this function
        template<class ConstraintClass, typename... Args>
        DLLEXPORT std::shared_ptr<ConstraintClass> CreateConstraintWith(Constraintable &other,
            Args&&... args)
        {
            auto tmpconstraint = std::make_shared<ConstraintClass>(World, *this, other, args...);

            if(tmpconstraint)
                _NotifyCreate(tmpconstraint, other);
            
            return tmpconstraint;
        }

        DLLEXPORT void RemoveConstraint(BaseConstraint* removed);

        DLLEXPORT void AddConstraint(std::shared_ptr<BaseConstraint> added);

    protected:

        //! \brief Notifies the other object and the GameWorld of the new constraint
        DLLEXPORT void _NotifyCreate(std::shared_ptr<BaseConstraint> newconstraint,
            Constraintable &other);

    public:
        
        
        std::vector<std::shared_ptr<BaseConstraint>> PartInConstraints;

        GameWorld* World;

        //! ID for other component lookup
        ObjectID PartOfEntity;
    };

    class Trail : public Component{
    public:

        struct ElementProperties{
            DLLEXPORT ElementProperties(const Float4 &initialcolour,
                const Float4 &colourchange, const float &initialsize, const float &sizechange) : 
                InitialColour(initialcolour), ColourChange(colourchange), InitialSize(initialsize),
                SizeChange(sizechange)
            {

            }
        
            DLLEXPORT ElementProperties(const Float4 &initialcolour,
                const float &initialsize) : 
                InitialColour(initialcolour), ColourChange(0), InitialSize(initialsize),
                SizeChange(0)
            {

            }

            DLLEXPORT ElementProperties() :
                InitialColour(1), ColourChange(0), InitialSize(1), SizeChange(0)
            {

            }

            Float4 InitialColour;
            Float4 ColourChange;
            float InitialSize;
            float SizeChange;
        };

        struct Properties{
        public:
            DLLEXPORT Properties(size_t maxelements, float lenght, float maxdistance,
                bool castshadows = false) :
                TrailLenght(lenght), MaxDistance(maxdistance),
                MaxChainElements(maxelements), CastShadows(castshadows), Elements(1)
            {
            }
        
            float TrailLenght;
            float MaxDistance;
            size_t MaxChainElements;
            bool CastShadows;

            std::vector<ElementProperties> Elements;
        };
        

    public:

        DLLEXPORT Trail(RenderNode* node, const std::string &materialname,
            const Properties &variables);

        //! \brief Sets properties on the trail object
        //! \pre Ogre objects have been created
        //! \param force If set to true all settings will be applied
		DLLEXPORT bool SetTrailProperties(const Properties &variables, bool force = false);

        //! \brief Destroys the TrailEntity
        DLLEXPORT void Release(Ogre::SceneManager* scene);

        //! The trail entity which is attached at the root scene node and follows our RenderNode
        //! component around
		Ogre::RibbonTrail* TrailEntity;

        //! For ease of use direct access to ogre node is required
        //! Not valid in non-gui mode
        RenderNode* _RenderNode;

        //! The used trail material
        std::string Material;

        //! Current settings, when updating settings only changed ones are applied
        Properties CurrentSettings;
    };

    //! \todo Add the Markers to the actual world for sending over the network individually
    class PositionMarkerOwner : public Component{
    public:

        struct Data{

            std::vector<std::tuple<ObjectID, Float3, Float4>> EntityPositions;
        };
        
    public:
        DLLEXPORT PositionMarkerOwner();

        //! \brief Create with automatically created positions
        DLLEXPORT PositionMarkerOwner(const Data &positions, GameWorld* world, Lock &worldlock);

        //! \brief Queues destruction and clears the list of Markers
        DLLEXPORT void Release(GameWorld* world, Lock &worldlock);

        //! Adds a node
        //! \todo Allow not deleting entities on release
        DLLEXPORT void Add(Lock &guard, ObjectID entity, Position& pos);

        DLLEXPORT inline void Add(ObjectID entity, Position& pos){

            GUARD_LOCK();
            Add(guard, entity, pos);
        }

        DLLEXPORT void AddDataToPacket(Lock &guard, sf::Packet &packet) const;

        DLLEXPORT inline void AddDataToPacket(sf::Packet &packet) const{

            GUARD_LOCK();
            AddDataToPacket(guard, packet);
        }

        //! \note The packet needs to be checked that it is valid after this call
        DLLEXPORT static Data LoadDataFromPacket(sf::Packet &packet);
        
        std::vector<std::tuple<ObjectID, Position*>> Markers;
    };

    class ManualObject : public Component{
    public:

        DLLEXPORT ManualObject();

        DLLEXPORT void Release(Ogre::SceneManager* scene);

        Ogre::ManualObject* Object;

        //! When not empty the ManualObject has been created into an actual mesh
        //! that needs to be destroyed on release
        std::string CreatedMesh;
    };

    //! \todo Split this into server and clientside components
    class TrackController : public Component{
    public:

        struct LocationNode{

            ObjectID Object;
            Position& Pos;
        };

        struct Arguments{

            PositionMarkerOwner& Nodes;
            Sendable* _Sendable;
            int ReachedNode;
            float NodeProgress;
            float ChangeSpeed;
            float ForceTowardsPoint;
        };

    public:

        DLLEXPORT TrackController(PositionMarkerOwner& nodes, Sendable* sendable);

        DLLEXPORT TrackController(const Arguments &args);

        //! \brief Runs update logic based on timestep
        //! \note Doesn't actually add applyforces only changes internal state
        //! \note This call can be avoided if ChangeSpeed == 0.f
        DLLEXPORT void Update(float timestep);

        DLLEXPORT bool Interpolate(ObjectDeltaStateData &first,
            ObjectDeltaStateData &second, float progress);

        //! \brief Internal function for making all data valid
        //!
        //! Checks for invalid reached node and progress
        void _SanityCheckNodeProgress(Lock &guard);

        //! \brief Returns the position and rotation for current NodeProgress
        DLLEXPORT void GetPositionOnTrack(Float3 &pos, Float4 &rot) const;

        //! \brief Retursn the position and rotation for the node at index
        DLLEXPORT bool GetNodePosition(int index, Float3 &pos, Float4 &rot) const;

        
        //! Number of the node that has been reached
        int ReachedNode;

        //! Percentage between ReachedNode and next node
        //! 1.f being next node reached and progress reset to 0
        float NodeProgress;

        //! The speed at which the node progress changes
        float ChangeSpeed;

        //! The amount of speed/force used to move the entities towards the track position
        float ForceTowardsPoint;

        //! Marks Sendable as updated when changed
        Sendable* _Sendable;

        //! Access to actual nodes
        PositionMarkerOwner& Nodes;
    };

    
}

using ApplyForceInfo = Leviathan::Physics::ApplyForceInfo;


