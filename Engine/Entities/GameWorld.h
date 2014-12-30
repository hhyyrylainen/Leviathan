#ifndef LEVIATHAN_GAMEWORLD
#define LEVIATHAN_GAMEWORLD
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "Objects/ViewerCameraPos.h"
#include "Newton/PhysicalWorld.h"
#include "Bases/BaseObject.h"
#include "Common/ReferenceCounted.h"
#include "Common/BaseNotifiable.h"


#define PHYSICS_BASE_GRAVITY		-9.81f

namespace Ogre{

	class CompositorWorkspace;
}

namespace Leviathan{


#define WORLD_CLOCK_SYNC_PACKETS 12
#define WORLD_CLOCK_SYNC_ALLOW_FAILS 2
#define WORLD_OBJECT_UPDATE_CLIENTS_INTERVAL 5

    //! Holds internal data for initial player syncing
    class PlayerConnectionPreparer;
    
	// Holds the returned object that was hit during ray casting //
	class RayCastHitEntity : public ReferenceCounted{
	public:
		DLLEXPORT RayCastHitEntity(const NewtonBody* ptr = NULL, const float &tvar = 0.f, RayCastData* ownerptr = NULL);

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

        struct WaitingConstraint{
            WaitingConstraint(int first, int second, shared_ptr<NetworkResponse> packet) :
                Entity1(first), Entity2(second), Packet(packet){}
            
            int Entity1, Entity2;
            shared_ptr<NetworkResponse> Packet;
        };
        
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


        //! \brief Used to keep track of passed ticks and trigger timed triggers
        //! \note This will be called (or should be) every time the engine ticks
        //!
        //! This will also advance the simulation time and simulate physics
        DLLEXPORT void Tick();


        //! \brief Returns the current tick
        DLLEXPORT int GetTickNumber() const;

        //! \brief Fetches the physical material ID from the material manager
        DLLEXPORT int GetPhysicalMaterial(const wstring &name);

		DLLEXPORT void SetFog();
		DLLEXPORT void SetSkyBox(const string &materialname);

		DLLEXPORT FORCE_INLINE void UpdateCameraLocation(int mspassed, ViewerCameraPos* camerapos){
			GUARD_LOCK_THIS_OBJECT();
			UpdateCameraLocation(mspassed, camerapos, guard);
		}
		DLLEXPORT void UpdateCameraLocation(int mspassed, ViewerCameraPos* camerapos, ObjectLock &guard);

		DLLEXPORT void SetSunlight();
		DLLEXPORT void RemoveSunlight();

		//! \brief Casts a ray from point along a vector and returns the first physical object it hits
		//! \warning You need to call Release on the returned object once done
		DLLEXPORT FORCE_INLINE RayCastHitEntity* CastRayGetFirstHit(const Float3 &from, const Float3 &to){
			GUARD_LOCK_THIS_OBJECT();
			return CastRayGetFirstHit(from, to, guard);
		}

		//! \brief Actual implementation of CastRayGetFirsHit
		DLLEXPORT RayCastHitEntity* CastRayGetFirstHit(const Float3 &from, const Float3 &to, ObjectLock &guard);


		// object managing functions //
		//! \brief Adds an existing entity to the world, which won't be broadcast to the world receiverd
        //! \see CreateEntity
		DLLEXPORT void AddObject(BaseObject* obj);

        //! \brief Adds a new entity
        //! \note This should be used instead of AddObject for most purposes
        //! \todo Allow to set the world to queue objects and send them in big bunches to players
        DLLEXPORT void CreateEntity(shared_ptr<BaseObject> obj);
        
        
		// The smart pointer should have custom deleter to use Release //
		DLLEXPORT void AddObject(shared_ptr<BaseObject> obj);
		DLLEXPORT void DestroyObject(int EntityID);
		DLLEXPORT void QueueDestroyObject(int EntityID);

		//! \brief Returns an object matching the id
		DLLEXPORT shared_ptr<BaseObject> GetWorldObject(int ID);

		//! \brief Returns a matching smart pointer for a raw pointer
		DLLEXPORT shared_ptr<BaseObject> GetSmartPointerForObject(BaseObject* rawptr) const;


		// clears all objects from the world //
		DLLEXPORT void ClearObjects(ObjectLock &guard);
		DLLEXPORT FORCE_INLINE void ClearObjects(){
			GUARD_LOCK_THIS_OBJECT();
			ClearObjects(guard);
		}

        //! \brief Returns the amount of entities in the world
        DLLEXPORT inline size_t GetObjectCount() const{
            return Objects.size();
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
        DLLEXPORT void ClearTimers();

        //! \brief Simulates physics
        DLLEXPORT void SimulatePhysics();

        //! \todo Synchronize this over the network
		DLLEXPORT void SetWorldPhysicsFrozenState(bool frozen);

		// Ray callbacks //
		static dFloat RayCallbackDataCallbackClosest(const NewtonBody* const body, const NewtonCollision* const
            shapeHit, const dFloat* const hitContact, const dFloat* const hitNormal, dLong collisionID, void* const
            userData, dFloat intersectParam);
		
		// Script proxies //
		DLLEXPORT RayCastHitEntity* CastRayGetFirstHitProxy(Float3 from, Float3 to);
		
		//! \brief Returns true when no players are marked as receiving initial update
		DLLEXPORT bool AreAllPlayersSynced() const;

        //! \brief Returns true when the player matching the connection should receive updates about an object
        //! \todo Implement this
        DLLEXPORT bool ShouldPlayerReceiveObject(BaseObject* obj, ConnectionInfo* connectionptr);

        //! \brief Sends an object to a connection and sets everything up
        //! \post The connection will receive updates from the object
        //! \param connection A safe pointer to the connection which won't be checked by this method
        //! \return True when a packet was sent false otherwise
        //! \todo Allow making these critical so that failing to send these will terminate the ConnectionInfo
        DLLEXPORT bool SendObjectToConnection(shared_ptr<BaseObject> obj, shared_ptr<ConnectionInfo> connection);
        
		//! \brief Creates a new entity from initial entity response
        //! \note This should only be called on the client
        DLLEXPORT bool HandleEntityInitialPacket(NetworkResponseDataForInitialEntity* data);

        //! \brief Applies a constraint packet
        //!
        //! If the entities aren't loaded yet the packet will be stored until they are
        DLLEXPORT void HandleConstraintPacket(NetworkResponseDataForEntityConstraint* data, shared_ptr<NetworkResponse>
            packet);

        //! \brief Applies an update packet
        //!
        //! If the entity is not found the packet is discarded
        //! \todo Cache the update data for 1 second and apply it if a matching entity is created during that time
        DLLEXPORT void HandleEntityUpdatePacket(NetworkResponseDataForEntityUpdate* data);

        //! \brief Handles a world clock synchronizing packet
        //! \note This should only be allowed to be called on a client that has connected to a server
        DLLEXPORT void HandleClockSyncPacket(RequestWorldClockSyncData* data);

        //! \brief Handles a world freeze/unfreeze packet
        //! \note Should only be called on a client
        DLLEXPORT void HandleWorldFrozenPacket(NetworkResponseDataForWorldFrozen* data);

        //! \brief Sends a Constraint to a connection
        //! \param constraint The constraint to send, the parent object needs to be locked during this call
        //! to avoid the constraint becoming invalid during this call
        //! \param connectionptr The connection to use, this must be a safe pointer
        DLLEXPORT void SendConstraintToConnection(shared_ptr<Entity::BaseConstraint> constraint,
            ConnectionInfo* connectionptr);
        
	private:

		//! Used to connect new players
        //! \todo Properly handle deleted and created objects (Potentially make objects vector have "empty"
        //! spaces in the middle
		DLLEXPORT virtual void _OnNotifiableConnected(BaseNotifiableAll* parentadded) override;

		//! Used to disconnect players that are going to be unloaded
		DLLEXPORT virtual void _OnNotifiableDisconnected(BaseNotifiableAll* parenttoremove) override;

		//! \brief Updates a players position info in this world
		void UpdatePlayersPositionData(ConnectedPlayer* ply, ObjectLock &guard);

		void _CreateOgreResources(Ogre::Root* ogre, Window* rendertarget);
		void _HandleDelayedDelete(UniqueObjectLock &guard);

        //! \brief Applies a constraint to entities, if both are present
        //! \returns True when the constraint is applied
        bool _TryApplyConstraint(NetworkResponseDataForEntityConstraint* data);

        //! \brief Removes a sendable entity from the specific sendable vector
        void _EraseFromSendable(BaseSendableEntity* obj, UniqueObjectLock &guard);

        //! \brief Reports an entity deletion to clients
        //! \todo Potentially send these in a big blob
        void _ReportEntityDestruction(int id, UniqueObjectLock &guard);
        

		// ------------------------------------ //
		Ogre::Camera* WorldSceneCamera;
		Ogre::SceneManager* WorldsScene;

		Ogre::CompositorWorkspace* WorldWorkspace;

		//! The world is now always linked to a window
		GraphicalInputEntity* LinkedToWindow;

		Ogre::Light* Sunlight;
		Ogre::SceneNode* SunLightNode;

		// physics //
		shared_ptr<PhysicalWorld> _PhysicalWorld;

		//! The world can be frozen to stop physics
		bool WorldFrozen;
		bool GraphicalMode;

		//! Marks all objects to be released
		bool ClearAllObjects;

		//! Holds the players who are receiving this worlds updates and their corresponding location entities (if any)
        //! \todo Change this to an object that holds more than the player pointer
		std::vector<ConnectedPlayer*> ReceivingPlayers;

        //! The constraints that are waiting for their entities to be created
        std::vector<WaitingConstraint> WaitingConstraints;
        

        //! This is not empty when some players are receiving their initial world state
        //! These objects need to be marked as invalid before quitting
        //! These can also be used to check whether all players have received
        //! the world
        std::vector<shared_ptr<PlayerConnectionPreparer>> InitiallySyncingPlayers;
        

		// objects //
		std::vector<shared_ptr<BaseObject>> Objects;

        //! Objects that are sendable and require additional operations
        std::vector<BaseSendableEntity*> SendableObjects;

        //! The unique ID
        int ID;

        //! The current tick number
        //! This should be the same on all clients as closely as possible
        int TickNumber;

        

        //! A funky name for this world, if any
        std::string DecoratedName;

        //! A lock for delayed delete, to allow deleting objects from physical callbacks
        boost::mutex DeleteMutex;
        
		//! This vector is used for delayed deletion
		std::vector<int> DelayedDeleteIDS;
	};

}
#endif
