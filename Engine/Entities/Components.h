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
        
        DLLEXPORT Sendable(SENDABLE_TYPE type);

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

        DLLEXPORT Received(SENDABLE_TYPE type);

        DLLEXPORT void GetServerSentStates(std::shared_ptr<ObjectDeltaStateData> &first,
            std::shared_ptr<ObjectDeltaStateData> &second, int tick, float &progress) const;

        //! Clientside buffer of past states
        boost::circular_buffer<StoredState> ClientStateBuffer;
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
        
        DLLEXPORT Physics(GameWorld* world, Position &updatepos, Sendable* updatesendable);

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
        DLLEXPORT Float3 GetVelocity(Lock &guard);

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

		bool Immovable;
		bool ApplyGravity;

        //! Non-newton access to mass
        float Mass;

		std::list<std::shared_ptr<ApplyForceInfo>> ApplyForceList;

        //! Used to access gravity data
        GameWorld* World;

        //! Physics object requires a position
        Position& _Position;

        // Optional access to other components that can be used for marking when physics object
        // moves
        Sendable* UpdateSendable;
    };

    class Parent : public Component{
    public:
        DLLEXPORT Parent();

    };

    class Parentable : public Component{
    public:
        DLLEXPORT Parentable();

        
        Float3 RelativeToParent;

        bool ApplyRotation;
    };


    class Constraintable : public Component{
    public:
        //! \param world Used to allow created constraints to access world data (including physics)
        DLLEXPORT Constraintable(GameWorld* world);

        //! Creates a constraint between this and another object
        //! \warning DO NOT store the returned value (since that reference isn't counted)
        //! \note Before the constraint is actually finished, you need to
        //! call ->SetParameters() on the returned ptr
        //! and then ->Init() and then let go of the ptr
        //! \note If you do not want to allow constraints where child is NULL you have to
        //! check if child is NULL before calling this function
        template<class ConstraintClass>
        DLLEXPORT std::shared_ptr<ConstraintClass> CreateConstraintWith(Constraintable &other){

            auto tmpconstraint = std::make_shared<ConstraintClass>(World, this, other);
            
            return tmpconstraint;
        }

        
        std::vector<std::shared_ptr<Entity::BaseConstraint>> PartInConstraints;

        GameWorld* World;
    };

    class PhysicsListener : public Component{
    public:
        DLLEXPORT PhysicsListener();

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
                Elements(1), TrailLenght(lenght), MaxDistance(maxdistance),
                CastShadows(castshadows), MaxChainElements(maxelements)
            {
            }
        
            float TrailLenght;
            float MaxDistance;
            size_t MaxChainElements;
            bool CastShadows;

            std::vector<ElementProperties> Elements;
        };
        

    public:

        Trail(RenderNode* node, const std::string &materialname, const Properties &variables);

        //! \brief Sets properties on the trail object
        //! \pre Ogre objects have been created
        //! \param force If set to true all settings will be applied
		DLLEXPORT bool SetTrailProperties(const Properties &variables, bool force = false);

        //! The trail entity which is attached at the root scene node and follows our RenderNode
        //! component around
		Ogre::RibbonTrail* TrailEntity;

        //! For ease of use direct access to ogre node is required
        //! Not valid in non-gui mode
        RenderNode* _RenderNode;

        //! Current settings, when updating settings only changed ones are applied
        Properties CurrentSettings;
    };
    
    class PositionMarkerOwner : public Component{
    public:
        DLLEXPORT PositionMarkerOwner();

        //! Adds a node
        //! \todo Allow not deleting entities on release
        DLLEXPORT void Add(ObjectID entity, Position& pos);

        
        std::vector<std::tuple<ObjectID, Position&>> Markers;
    };

    class ManualObject : public Component{
    public:

        ManualObject(const std::string &meshname = "");

        Ogre::ManualObject* Object;

        //! When not empty the ManualObject has been created into an actual mesh
        //! that needs to be destroyed on release
        std::string CreatedMesh;
    };

    class TrackController : public Component{
    public:

        struct LocationNode{

            ObjectID Object;
            Position& Pos;
        };

    public:

        DLLEXPORT TrackController();

        //! \brief Directly sets the progress towards next node (if set to 1.f goes to next node)
        DLLEXPORT void SetProgressTowardsNextNode(float progress);
        
        //! \brief Gets the progress towards next node, if at 1.f then last node is reached
        DLLEXPORT float GetProgressTowardsNextNode(){
            return NodeProgress;
        }

        //! \brief Controls the speed at which the entity moves along the track
        //! (set to negative to go backwards and 0.f to stop)
        DLLEXPORT void SetTrackAdvanceSpeed(const float &speed);
            
        DLLEXPORT inline float GetTrackAdvanceSpeed(){
            return ChangeSpeed;
        }

        //! \brief Updates the entity positions
        //! \note You probably don't have to manually call this
        DLLEXPORT virtual void UpdateControlledPositions(float timestep);

        DLLEXPORT bool SetStateToInterpolated(ObjectDeltaStateData &first,
            ObjectDeltaStateData &second, float progress);

        //! \brief Internal function for making all data valid
        //!
        //! Checks for invalid reached node and progress
        void _SanityCheckNodeProgress(Lock &guard);

        
        //! Number of the node that has been reached
        int ReachedNode;

        //! Percentage between ReachedNode and next node
        //! 1.f being next node reached and progress reset to 0
        float NodeProgress;

        //! The speed at which the node progress changes
        float ChangeSpeed;

        //! The amount of speed/force used to move the entities towards the track position
        float ForceTowardsPoint;
    };

    
}
