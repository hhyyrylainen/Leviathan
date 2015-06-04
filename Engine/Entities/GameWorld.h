#pragma once
// ------------------------------------ //
#include "Define.h"
// ------------------------------------ //
#include <type_traits>

#include "Systems.h"
#include "Components.h"
#include "Nodes.h"

#include "Objects/ViewerCameraPos.h"
#include "../Newton/PhysicalWorld.h"
#include "../Common/ReferenceCounted.h"
#include "../Common/BaseNotifiable.h"


#define PHYSICS_BASE_GRAVITY		-9.81f

namespace Ogre{

	class CompositorWorkspace;
}

namespace Leviathan{


#define WORLD_CLOCK_SYNC_PACKETS 12
#define WORLD_CLOCK_SYNC_ALLOW_FAILS 2
#define WORLD_OBJECT_UPDATE_CLIENTS_INTERVAL 2
    

    //! Holds internal data for initial player syncing
    class PlayerConnectionPreparer;
    
	// Holds the returned object that was hit during ray casting //
	class RayCastHitEntity : public ReferenceCounted{
	public:
		DLLEXPORT RayCastHitEntity(const NewtonBody* ptr = NULL, const float &tvar = 0.f,
            RayCastData* ownerptr = NULL);

		DLLEXPORT RayCastHitEntity& operator =(const RayCastHitEntity& other);

		// Compares the hit entity with NULL //
		DLLEXPORT bool HasHit();

		DLLEXPORT Float3 GetPosition();

		REFERENCECOUNTED_ADD_PROXIESFORANGELSCRIPT_DEFINITIONS(RayCastHitEntity);

		DLLEXPORT bool DoesBodyMatchThisHit(NewtonBody* other);

		// Stores the entity, typed as NewtonBody to make sure that user knows what should be compared with this //
		const NewtonBody* HitEntity;
		float HitVariable;
		Float3 HitLocation;
	};

	// Internal object in ray casts //
	struct RayCastData{
		DLLEXPORT RayCastData(int maxcount, const Float3 &from, const Float3 &to);
		DLLEXPORT ~RayCastData();

		// All hit entities that pass checks //
		std::vector<RayCastHitEntity*> HitEntities;
		// Used to stop after certain amount of entities found //
		int MaxCount;
		// Used to efficiently calculate many hit locations //
		Float3 BaseHitLocationCalcVar;
	};

	//! \brief Represents a world that contains entities
    //! \note Only ConnectedPlayer object may be linked with the world through Notifier
	class GameWorld : public BaseNotifierAll{
        friend PlayerConnectionPreparer;
	public:
		DLLEXPORT GameWorld();
		DLLEXPORT ~GameWorld();


        //! \brief Returns the unique ID of this world
        DLLEXPORT inline int GetID() const{
            return ID;
        }

		//! \brief Creates resources for the world to work
		//! \post The world can be used after this
		DLLEXPORT bool Init(GraphicalInputEntity* renderto, Ogre::Root* ogre);

		//! Release to not use Ogre when deleting
		DLLEXPORT void Release();

		//! \brief Marks all objects to be deleted
		DLLEXPORT void MarkForClear();

        // clears all objects from the world //
		DLLEXPORT void ClearObjects(Lock &guard);

        //! \brief Returns the number of ObjectIDs this world keeps track of
        //! \note There may actually be more objects as it is possible to create components
        //! for ids that are not created
        DLLEXPORT int GetObjectCount() const;


        //! \brief Used to keep track of passed ticks and trigger timed triggers
        //! \note This will be called (or should be) every time the engine ticks
        //! \note This cannot be used for accurate time keeping for that use timers, but for
        //! events that need to happen at certain game world times this is ideal
        DLLEXPORT void Tick(int currenttick);

        //! \brief Returns the current tick
        DLLEXPORT int GetTickNumber() const;

        //! \brief Handles deleted entities
        DLLEXPORT void HandleDeleted(Lock &guard);

        //! \brief Destroyes nodes that no longer have their required components available
        DLLEXPORT void RemoveInvalidNodes(Lock &guard);

        //! \brief Handles added entities and components
        DLLEXPORT void HandleAdded(Lock &guard);

        //! \brief Called by engine before frame rendering
        //! \todo Only call on worlds that contain cameras that are connected
        //! to GraphicalInputEntities
        DLLEXPORT void RunFrameRenderSystems(int timeintick);
        

        //! \brief Fetches the physical material ID from the material manager
        DLLEXPORT int GetPhysicalMaterial(const std::string &name);

		DLLEXPORT void SetFog();
		DLLEXPORT void SetSkyBox(const std::string &materialname);

        DLLEXPORT void SetSunlight();
		DLLEXPORT void RemoveSunlight();

		DLLEXPORT FORCE_INLINE void UpdateCameraLocation(int mspassed, ViewerCameraPos* camerapos){
			GUARD_LOCK();
			UpdateCameraLocation(mspassed, camerapos, guard);
		}
        
		DLLEXPORT void UpdateCameraLocation(int mspassed, ViewerCameraPos* camerapos,
            Lock &guard);


		//! \brief Casts a ray from point along a vector and returns the first physical
        //! object it hits
		//! \warning You need to call Release on the returned object once done
		DLLEXPORT FORCE_INLINE RayCastHitEntity* CastRayGetFirstHit(const Float3 &from,
            const Float3 &to)
        {
			GUARD_LOCK();
			return CastRayGetFirstHit(from, to, guard);
		}

		//! \brief Actual implementation of CastRayGetFirsHit
		DLLEXPORT RayCastHitEntity* CastRayGetFirstHit(const Float3 &from, const Float3 &to,
            Lock &guard);


        //! \brief Creates a new empty entity and returns its id
        DLLEXPORT ObjectID CreateEntity(Lock &guard);

        DLLEXPORT inline ObjectID CreateEntity(){

            GUARD_LOCK();
            return CreateEntity(guard);
        }

        //! \brief Destroys an entity and all of its components
        //! \todo Make this less expensive
        DLLEXPORT void DestroyObject(ObjectID id);

        //! \brief Deletes an entity during the next tick
        DLLEXPORT void QueueDestroyObject(ObjectID id);

        //! \brief Notifies others that we have created a new entity
        //! \note This is called after all components are set up and it is ready to be sent to
        //! other players
        //! \note Clients should also call this function
        //! \todo Allow to set the world to queue objects and send them in big bunches to players
        DLLEXPORT void NotifyEntityCreate(Lock &guard, ObjectID id);

        DLLEXPORT inline void NotifyEntityCreate(ObjectID id){

            GUARD_LOCK();
            NotifyEntityCreate(guard, id);
        }


        //! \brief Returns a reference to a component of wanted type
        //! \exception NotFound when the specified entity doesn't have a component of the wanted
        //! type
        template<class ComponentType>
        DLLEXPORT ComponentType& GetComponent(ObjectID id){
            
            static_assert(std::is_same<ComponentType, std::false_type>::value,
                "Trying to use a component type that is missing a template specialization");
        }

        //! \brief Destroys a component belonging to an entity
        //! \return True when destroyed, false if the entity didn't have a component of this type
        template<class ComponentType>
        DLLEXPORT bool RemoveComponent(ObjectID id){

            static_assert(std::is_same<ComponentType, std::false_type>::value,
                "Trying to use a component type that is missing a template specialization");
            return false;
        }

        //! \brief Creates a new component for entity
        //! \exception Exception if the component failed to init or it already exists
        template<typename... Args>
        DLLEXPORT Position& CreatePosition(ObjectID id, Args&&... args){

            return *ComponentPosition.ConstructNew(id, args...);
        }

        //! \brief Creates a new component for entity
        //! \exception Exception if the component failed to init or it already exists
        template<typename... Args>
        DLLEXPORT RenderNode& CreateRenderNode(ObjectID id, Args&&... args){

            return *ComponentRenderNode.ConstructNew(id, args...);
        }

        template<typename... Args>
        DLLEXPORT Sendable& CreateSendable(ObjectID id, Args&&... args){

            return *ComponentSendable.ConstructNew(id, args...);
        }

        template<typename... Args>
        DLLEXPORT Model& CreateModel(ObjectID id, Args&&... args){

            return *ComponentModel.ConstructNew(id, args...);
        }

        template<typename... Args>
        DLLEXPORT Physics& CreatePhysics(ObjectID id, Args&&... args){

            const Physics::Arguments createdargs = {args...};
            return *ComponentPhysics.ConstructNew(id, createdargs);
        }

        template<typename... Args>
        DLLEXPORT Constraintable& CreateConstraintable(ObjectID id, Args&&... args){

            return *ComponentConstraintable.ConstructNew(id, args...);
        }

        template<typename... Args>
        DLLEXPORT BoxGeometry& CreateBoxGeometry(ObjectID id, Args&&... args){

            return *ComponentBoxGeometry.ConstructNew(id, args...);
        }

        template<typename... Args>
        DLLEXPORT ManualObject& CreateManualObject(ObjectID id, Args&&... args){

            return *ComponentManualObject.ConstructNew(id, args...);
        }

        template<typename... Args>
        DLLEXPORT PositionMarkerOwner& CreatePositionMarkerOwner(ObjectID id, Args&&... args){

            return *ComponentPositionMarkerOwner.ConstructNew(id, args...);
        }

        template<typename... Args>
        DLLEXPORT Parent& CreateParent(ObjectID id, Args&&... args){

            return *ComponentParent.ConstructNew(id, args...);
        }

        template<typename... Args>
        DLLEXPORT Parentable& CreateParentable(ObjectID id, Args&&... args){

            return *ComponentParentable.ConstructNew(id, args...);
        }        

        template<typename... Args>
        DLLEXPORT Trail& CreateTrail(ObjectID id, Args&&... args){

            return *ComponentTrail.ConstructNew(id, args...);
        }

        template<typename... Args>
        DLLEXPORT TrackController& CreateTrackController(ObjectID id, Args&&... args){

            const TrackController::Arguments params = { args... };
            return *ComponentTrackController.ConstructNew(id, params);
        }

        template<typename... Args>
        DLLEXPORT Received& CreateReceived(ObjectID id, Args&&... args){

            return *ComponentReceived.ConstructNew(id, args...);
        }

        // Systems //
        template<typename... Args>
        DLLEXPORT void RunRenderingPositionSystem(Args&&... args){

            NodeRenderingPosition.RunSystem(_RenderingPositionSystem, args...);
        }

        template<typename... Args>
        DLLEXPORT void RunSendableSystem(Args&&... args){

            NodeSendableNode.RunSystem(_SendableSystem, args...);
        }

        template<typename... Args>
        DLLEXPORT void RunInterpolationSystem(Args&&... args){

            NodeReceivedPosition.RunSystem(_ReceivedPositionSystem, args...);
        }

		// Ogre get functions //
		DLLEXPORT inline Ogre::SceneManager* GetScene(){
			return WorldsScene;
		}
        
		// physics functions //
		DLLEXPORT Float3 GetGravityAtPosition(const Float3 &pos);

		DLLEXPORT inline PhysicalWorld* GetPhysicalWorld(){
			return _PhysicalWorld.get();
		}

        //! \brief Resets physical timers
        DLLEXPORT void ClearTimers(Lock &guard);

        DLLEXPORT inline void ClearTimers(){
            
            GUARD_LOCK();
            ClearTimers(guard);
        }
        
        //! \brief Simulates physics
        DLLEXPORT void SimulatePhysics(Lock &guard);

        DLLEXPORT inline void SimulatePhysics(){

            GUARD_LOCK();
            SimulatePhysics(guard);
        }

        //! \todo Synchronize this over the network
		DLLEXPORT void SetWorldPhysicsFrozenState(Lock &guard, bool frozen);

        DLLEXPORT inline void SetWorldPhysicsFrozenState(bool frozen){

            GUARD_LOCK();
            SetWorldPhysicsFrozenState(guard, frozen);
        }

        //! \brief Call when a new constraint is created, will broadcast on the server
        DLLEXPORT void NotifyNewConstraint(std::shared_ptr<BaseConstraint> constraint);

        //! \brief Removes a constraint and notifies possible clients that it was destroyed
        DLLEXPORT void ConstraintDestroyed(BaseConstraint* constraint);

        //! \brief Creates a network constraint at the next suitable time
        DLLEXPORT bool HandleConstraintPacket(NetworkResponseDataForEntityConstraint* data);
        

		// Ray callbacks //
		static dFloat RayCallbackDataCallbackClosest(const NewtonBody* const body,
            const NewtonCollision* const shapeHit, const dFloat* const hitContact,
            const dFloat* const hitNormal, dLong collisionID, void* const userData,
            dFloat intersectParam);
		
		// Script proxies //
		DLLEXPORT RayCastHitEntity* CastRayGetFirstHitProxy(const Float3 &from, const Float3 &to);
		
		//! \brief Returns true when no players are marked as receiving initial update
		DLLEXPORT bool AreAllPlayersSynced() const;

        //! \brief Returns true when the player matching the connection should receive updates
        //! about an object
        //! \todo Implement this
        DLLEXPORT bool ShouldPlayerReceiveObject(Position &atposition,
            ConnectionInfo* connectionptr);

        //! \brief Sends an object to a connection and sets everything up
        //! \post The connection will receive updates from the object
        //! \param connection A safe pointer to the connection which won't be checked
        //! \return True when a packet was sent false otherwise
        //! \todo Allow making these critical so that failing to send these will terminate
        //! the ConnectionInfo
        DLLEXPORT bool SendObjectToConnection(Lock &guard, ObjectID obj,
            std::shared_ptr<ConnectionInfo> connection);
        
		//! \brief Creates a new entity from initial entity response
        //! \note This should only be called on the client
        DLLEXPORT bool HandleEntityInitialPacket(NetworkResponseDataForInitialEntity* data);

        //! \brief Applies an update packet
        //!
        //! If the entity is not found the packet is discarded
        //! \todo Cache the update data for 1 second and apply it if a matching entity is
        //! created during that time
        DLLEXPORT void HandleEntityUpdatePacket(NetworkResponseDataForEntityUpdate* data);

        //! \brief Handles a world clock synchronizing packet
        //! \note This should only be allowed to be called on a client that has connected
        //! to a server
        DLLEXPORT void HandleClockSyncPacket(RequestWorldClockSyncData* data);

        //! \brief Handles a world freeze/unfreeze packet
        //! \note Should only be called on a client
        DLLEXPORT void HandleWorldFrozenPacket(NetworkResponseDataForWorldFrozen* data);

	private:

		//! Used to connect new players
        //! \todo Properly handle deleted and created objects
        //! (Potentially make objects vector have "empty" spaces in the middle)
		DLLEXPORT virtual void _OnNotifiableConnected(Lock &guard,
            BaseNotifiableAll* parentadded, Lock &parentlock) override;

		//! Used to disconnect players that are going to be unloaded
		DLLEXPORT virtual void _OnNotifiableDisconnected(Lock &guard,
            BaseNotifiableAll* parenttoremove, Lock &parentlock) override;

		//! \brief Updates a players position info in this world
		void UpdatePlayersPositionData(Lock &guard, ConnectedPlayer* ply, Lock &plylock);

		void _CreateOgreResources(Ogre::Root* ogre, Window* rendertarget);
		void _HandleDelayedDelete(Lock &guard);

        //! \brief Reports an entity deletion to clients
        //! \todo Potentially send these in a big blob
        void _ReportEntityDestruction(Lock &guard, ObjectID id);

        //! \brief Implementation of doing actual destroy part of removing an entity
        void _DoDestroy(Lock &guard, ObjectID id);

        //! \brief Sends sendable updates to all clients
        void _SendEntityUpdates(Lock &guard, ObjectID id, Sendable &sendable, int tick);

		// ------------------------------------ //
		Ogre::Camera* WorldSceneCamera;
		Ogre::SceneManager* WorldsScene;

		Ogre::CompositorWorkspace* WorldWorkspace;

		//! The world is now always linked to a window
		GraphicalInputEntity* LinkedToWindow;

		Ogre::Light* Sunlight;
		Ogre::SceneNode* SunLightNode;

		// physics //
        std::shared_ptr<PhysicalWorld> _PhysicalWorld;

		//! The world can be frozen to stop physics
		bool WorldFrozen;
		bool GraphicalMode;

		//! Marks all objects to be released
		bool ClearAllObjects;

		//! Holds the players who are receiving this worlds updates and their corresponding
        //! location entities (if any)
        //! \todo Change this to an object that holds more than the player pointer
		std::vector<ConnectedPlayer*> ReceivingPlayers;

        //! This is not empty when some players are receiving their initial world state
        //! These objects need to be marked as invalid before quitting
        //! These can also be used to check whether all players have received
        //! the world
        std::vector<std::shared_ptr<PlayerConnectionPreparer>> InitiallySyncingPlayers;

		// objects //
		std::vector<ObjectID> Objects;

        //! The unique ID
        int ID;

        //! Bool flag telling whether this is a master world (on a server) or
        //! a mirroring world (client)
        bool IsOnServer;

        //! The current tick number
        //! This should be the same on all clients as closely as possible
        int TickNumber;

        //! A funky name for this world, if any
        std::string DecoratedName;

        //! A lock for delayed delete, to allow deleting objects from physical callbacks
        Mutex DeleteMutex;
        
		//! This vector is used for delayed deletion
		std::vector<ObjectID> DelayedDeleteIDS;

        // Has IDs of deleted objects is used to destroy nodes
        std::vector<ObjectID> NodesToInvalidate;

        //! Mutex for ConstraintList
        Mutex ConstraintListMutex;

        //! List of constraints in this world
        //!
        //! Used to send full lists to clients
        std::vector<std::shared_ptr<BaseConstraint>> ConstraintList;

        // Systems, nodes and components //
        // Note: all of these should be cleared in ClearObjects
        ComponentHolder<Position> ComponentPosition;
        ComponentHolder<RenderNode> ComponentRenderNode;
        ComponentHolder<Sendable> ComponentSendable;
        ComponentHolder<Model> ComponentModel;
        ComponentHolder<Physics> ComponentPhysics;
        ComponentHolder<Constraintable> ComponentConstraintable;
        ComponentHolder<BoxGeometry> ComponentBoxGeometry;
        ComponentHolder<ManualObject> ComponentManualObject;
        ComponentHolder<PositionMarkerOwner> ComponentPositionMarkerOwner;
        ComponentHolder<Parent> ComponentParent;
        ComponentHolder<Trail> ComponentTrail;
        ComponentHolder<TrackController> ComponentTrackController;
        ComponentHolder<Received> ComponentReceived;
        ComponentHolder<Parentable> ComponentParentable;

        // Systems and nodes //

        NodeHolder<ReceivedPosition> NodeReceivedPosition;
        ReceivedPositionSystem _ReceivedPositionSystem;
        
        NodeHolder<RenderingPosition> NodeRenderingPosition;
        RenderingPositionSystem _RenderingPositionSystem;

        NodeHolder<SendableNode> NodeSendableNode;
        SendableSystem _SendableSystem;

	};

#define ADDCOMPONENTFUNCTIONSTOGAMEWORLD(type, holder, destroyfunc) template<> type& \
    GameWorld::GetComponent<type>(ObjectID id);\
                                                                        \
    template<> bool GameWorld::RemoveComponent<type>(ObjectID id);
    

    ADDCOMPONENTFUNCTIONSTOGAMEWORLD(Position, ComponentPosition, Destroy);
    ADDCOMPONENTFUNCTIONSTOGAMEWORLD(RenderNode, ComponentRenderNode, QueueDestroy);
    ADDCOMPONENTFUNCTIONSTOGAMEWORLD(Sendable, ComponentSendable, Destroy);
    ADDCOMPONENTFUNCTIONSTOGAMEWORLD(Physics, ComponentPhysics, QueueDestroy);
    ADDCOMPONENTFUNCTIONSTOGAMEWORLD(BoxGeometry, ComponentBoxGeometry, Destroy);
    ADDCOMPONENTFUNCTIONSTOGAMEWORLD(Model, ComponentModel, QueueDestroy);
    ADDCOMPONENTFUNCTIONSTOGAMEWORLD(TrackController, ComponentTrackController, Destroy);
    ADDCOMPONENTFUNCTIONSTOGAMEWORLD(Parent, ComponentParent, Destroy);
    ADDCOMPONENTFUNCTIONSTOGAMEWORLD(Parentable, ComponentParentable, Destroy);
    ADDCOMPONENTFUNCTIONSTOGAMEWORLD(PositionMarkerOwner, ComponentPositionMarkerOwner,
        QueueDestroy);
    ADDCOMPONENTFUNCTIONSTOGAMEWORLD(Received, ComponentReceived, Destroy);
    ADDCOMPONENTFUNCTIONSTOGAMEWORLD(Constraintable, ComponentConstraintable, Destroy);
    ADDCOMPONENTFUNCTIONSTOGAMEWORLD(Trail, ComponentTrail, QueueDestroy);
    ADDCOMPONENTFUNCTIONSTOGAMEWORLD(ManualObject, ComponentManualObject, QueueDestroy);
    
}

