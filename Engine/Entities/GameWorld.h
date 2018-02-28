// Leviathan Game Engine
// Copyright (c) 2012-2017 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "Common/ReferenceCounted.h"
#include "Common/ThreadSafe.h"
#include "Component.h"
#include "Networking/CommonNetwork.h"

#include <type_traits>

#define PHYSICS_BASE_GRAVITY -9.81f

class NewtonBody;

class CScriptArray;
class asIScriptObject;
class asIScriptFunction;

namespace Ogre {

class CompositorWorkspace;
class Plane;
} // namespace Ogre

namespace Leviathan {

class Camera;
class PhysicalWorld;
class ScriptComponentHolder;

template<class StateT>
class StateHolder;

struct ComponentTypeInfo {

    inline ComponentTypeInfo(uint16_t ltype, int astype) :
        LeviathanType(ltype), AngelScriptType(astype)
    {
    }

    uint16_t LeviathanType;
    int AngelScriptType;
};


#define WORLD_CLOCK_SYNC_PACKETS 12
#define WORLD_CLOCK_SYNC_ALLOW_FAILS 2
#define WORLD_OBJECT_UPDATE_CLIENTS_INTERVAL 2

//! Holds the returned entity that was hit during ray casting
//! \todo Move to a new file
class RayCastHitEntity : public ReferenceCounted {
public:
    DLLEXPORT RayCastHitEntity(const NewtonBody* ptr = nullptr, const float& tvar = 0.f,
        RayCastData* ownerptr = nullptr);

    DLLEXPORT RayCastHitEntity& operator=(const RayCastHitEntity& other);

    // Compares the hit entity with nullptr //
    DLLEXPORT bool HasHit();

    DLLEXPORT Float3 GetPosition();

    DLLEXPORT bool DoesBodyMatchThisHit(NewtonBody* other);

    //! Stores the entity, typed as NewtonBody to make sure that user knows
    //! what should be compared with this
    const NewtonBody* HitEntity;

    //! The distance from the start of the ray to the hit location
    float HitVariable;
    Float3 HitLocation;
};

// Internal object in ray casts //
struct RayCastData {
    DLLEXPORT RayCastData(int maxcount, const Float3& from, const Float3& to);
    DLLEXPORT ~RayCastData();

    // All hit entities that pass checks //
    std::vector<RayCastHitEntity*> HitEntities;
    // Used to stop after certain amount of entities found //
    int MaxCount;
    // Used to efficiently calculate many hit locations //
    Float3 BaseHitLocationCalcVar;
};

//! \brief Represents a world that contains entities
//!
//! This is the base class from which worlds that support different components are derived from
//! Custom worls should derive from StandardWorld which has all of the standard components
//! supported. See the GenerateStandardWorld.rb file to figure out how to generate world
//! classes
class GameWorld {
    class Implementation;

public:
    DLLEXPORT GameWorld();
    DLLEXPORT ~GameWorld();

    //! \brief Returns the unique ID of this world
    DLLEXPORT inline int GetID() const
    {
        return ID;
    }

    //! \brief Creates resources for the world to work
    //! \post The world can be used after this
    DLLEXPORT bool Init(NETWORKED_TYPE type, GraphicalInputEntity* renderto, Ogre::Root* ogre);

    //! Release to not use Ogre when deleting
    DLLEXPORT void Release();

    //! \brief Marks all entities to be deleted
    DLLEXPORT void MarkForClear();

    //! Clears all objects from the world
    DLLEXPORT void ClearEntities();

    //! \brief Returns the number of ObjectIDs this world keeps track of
    //! \note There may actually be more entities as it is possible to create components
    //! for ids that are not created (this is not recommended but it isn't enforced)
    DLLEXPORT size_t GetEntityCount() const
    {
        return Entities.size();
    }


    //! \brief Used to keep track of passed ticks and trigger timed triggers
    //! \note This will be called (or should be) every time the engine ticks
    //! \note This cannot be used for accurate time keeping for that use timers, but for
    //! events that need to happen at certain game world times this is ideal
    DLLEXPORT void Tick(int currenttick);

    //! \brief Runs systems required for a rendering run. Also updates camera positions
    //! \todo Allow script systems to specify their type
    DLLEXPORT void Render(int mspassed, int tick, int timeintick);

    //! \brief Returns the current tick
    DLLEXPORT int GetTickNumber() const;

    //! \brief Returns float between 0.f and 1.f based on how far current tick has progressed
    DLLEXPORT float GetTickProgress() const;

    //! \brief Returns a tuple of the current tick number and how long
    //! it has passed since last tick
    //!
    //! The tick number is always adjusted so that the time since last tick is < TICKSPEED
    DLLEXPORT std::tuple<int, int> GetTickAndTime() const;


    //! \brief Fetches the physical material ID from the material manager
    DLLEXPORT int GetPhysicalMaterial(const std::string& name);


    //! \brief Casts a ray from point along a vector and returns the first physical
    //! object it hits
    //! \warning You need to call Release on the returned object once done
    DLLEXPORT RayCastHitEntity* CastRayGetFirstHit(const Float3& from, const Float3& to);

    //! \brief Creates a new empty entity and returns its id
    //! \todo Make this have a per world counter
    DLLEXPORT ObjectID CreateEntity();

    //! \brief Destroys an entity and all of its components
    //! \warning This destroyes the entity immediately. If called during a system update this
    //! will cause issues as required components may be destroyed and cached components will
    //! only be updated at the start of next tick. So use QueueDestroyEntity instead. \todo
    //! Make this less expensive
    DLLEXPORT void DestroyEntity(ObjectID id);

    //! \brief Deletes an entity during the next tick
    DLLEXPORT void QueueDestroyEntity(ObjectID id);

    //! \brief Makes child entity be deleted when parent is deleted
    //! \note Doesn't check that the entitiy ids exist
    DLLEXPORT void SetEntitysParent(ObjectID child, ObjectID parent);

    //! \brief Notifies others that we have created a new entity
    //! \note This is called after all components are set up and it is ready to be sent to
    //! other players
    //! \note Clients should also call this function
    //! \todo Allow to set the world to queue objects and send them in
    //! big bunches to players
    DLLEXPORT void NotifyEntityCreate(ObjectID id);


    //! \brief Removes all components from an entity
    DLLEXPORT virtual void DestroyAllIn(ObjectID id);

    //! Helper for getting component of type. This is much slower than
    //! direct lookups with the actual implementation class' GetComponent_Position etc.
    //! methods
    //! \exception NotFound if entity has no component of the wanted type
    template<class TComponent>
    TComponent& GetComponent(ObjectID id)
    {

        std::tuple<void*, bool> component = GetComponent(id, TComponent::TYPE);

        if(!std::get<1>(component))
            throw InvalidArgument("Unrecognized component type as template parameter");

        void* ptr = std::get<0>(component);

        if(!ptr)
            throw NotFound("Component for entity with id was not found");

        return *static_cast<TComponent*>(ptr);
    }

    //! \brief Gets a component of type or returns nullptr
    //!
    //! \returns Tuple of pointer to component and boolean indicating if the type is known
    DLLEXPORT virtual std::tuple<void*, bool> GetComponent(ObjectID id, COMPONENT_TYPE type);

    //! \brief Gets a component of type or returns nullptr
    //!
    //! \returns Tuple of pointer to component and boolean indicating if the type is known
    DLLEXPORT virtual std::tuple<void*, ComponentTypeInfo, bool> GetComponentWithType(
        ObjectID id, COMPONENT_TYPE type);

    //! Helper for getting component state holder for type. This is much slower than
    //! direct lookups with the actual implementation class' GetStatesFor_Position etc.
    //! methods
    //! \exception NotFound if entity has no component of the wanted type
    template<class TComponent>
    StateHolder<typename TComponent::StateT>& GetStatesFor()
    {

        std::tuple<void*, bool> stateHolder = GetStatesFor(TComponent::TYPE);

        if(!std::get<1>(stateHolder))
            throw InvalidArgument("Unrecognized component type as template parameter for "
                                  "state holder");

        void* ptr = std::get<0>(stateHolder);

        return *static_cast<StateHolder<typename TComponent::StateT>*>(ptr);
    }

    //! \brief Gets a component states of type or returns nullptr
    //!
    //! \returns Tuple of pointer to component and boolean indicating if the type is known
    DLLEXPORT virtual std::tuple<void*, bool> GetStatesFor(COMPONENT_TYPE type);

    //! \brief Gets a list of destroyed components of type
    //!
    //! \returns True if the type is known and no further base classes need to be checked
    DLLEXPORT virtual bool GetRemovedFor(
        COMPONENT_TYPE type, std::vector<std::tuple<void*, ObjectID>>& result);

    //! \brief Variant of GetRemovedFor for script defined types
    DLLEXPORT bool GetRemovedForScriptDefined(
        const std::string& name, std::vector<std::tuple<asIScriptObject*, ObjectID>>& result);

    //! \brief Gets a list of created components of type
    //! \see GetRemovedFor
    //! \todo Find a better way than having to have the component type specified here. This is
    //! only used by script node proxy, so this has to be somewhere for it to access
    DLLEXPORT virtual bool GetAddedFor(COMPONENT_TYPE type,
        std::vector<std::tuple<void*, ObjectID, ComponentTypeInfo>>& result);

    //! \brief Variant of GetAddedFor for script defined types
    DLLEXPORT bool GetAddedForScriptDefined(const std::string& name,
        std::vector<std::tuple<asIScriptObject*, ObjectID, ScriptComponentHolder*>>& result);

    //! \brief Sets the entity that acts as a camera.
    //!
    //! The entity needs atleast Position and Camera components
    //! \exception InvalidArgument if the object is missing required components
    DLLEXPORT void SetCamera(ObjectID object);

    // Ogre get functions //
    inline Ogre::SceneManager* GetScene()
    {
        return WorldsScene;
    }

    // physics functions //
    DLLEXPORT Float3 GetGravityAtPosition(const Float3& pos);

    inline PhysicalWorld* GetPhysicalWorld()
    {
        return _PhysicalWorld.get();
    }

    //! \todo Synchronize this over the network
    DLLEXPORT void SetWorldPhysicsFrozenState(bool frozen);

    // Script proxies //
    DLLEXPORT RayCastHitEntity* CastRayGetFirstHitProxy(const Float3& from, const Float3& to);

    //! \brief Returns true when the player matching the connection should receive updates
    //! about an entity
    //! \todo Implement this
    DLLEXPORT bool ShouldPlayerReceiveEntity(Position& atposition, Connection& connection);

    //! \brief Returns true if a player with the given connection is receiving updates for
    //! this world
    DLLEXPORT bool IsConnectionInWorld(Connection& connection) const;

    //! \brief Verifies that player is receiving this world
    DLLEXPORT void SetPlayerReceiveWorld(std::shared_ptr<ConnectedPlayer> ply);

    //! \brief Sends a packet to all connected players
    DLLEXPORT void SendToAllPlayers(
        const std::shared_ptr<NetworkResponse>& response, RECEIVE_GUARANTEE guarantee) const;

    //! \brief Sends an entity to a connection and sets everything up
    //! \post The connection will receive updates from the entity
    //! \return True when a packet was sent false otherwise
    DLLEXPORT bool SendEntityToConnection(
        ObjectID obj, std::shared_ptr<Connection> connection);

    //! \brief Creates a new entity from initial entity response
    //! \note This should only be called on the client
    DLLEXPORT void HandleEntityInitialPacket(
        std::shared_ptr<NetworkResponse> message, ResponseEntityCreation* data);

    //! \brief Applies an update packet
    //!
    //! If the entity is not found the packet is discarded
    //! \todo Cache the update data for 1 second and apply it if a matching entity is
    //! created during that time
    DLLEXPORT void HandleEntityUpdatePacket(std::shared_ptr<NetworkResponse> message);

    //! \brief Handles a world clock synchronizing packet
    //! \note This should only be allowed to be called on a client that has connected
    //! to a server
    DLLEXPORT void HandleClockSyncPacket(RequestWorldClockSync* data);

    //! \brief Handles a world freeze/unfreeze packet
    //! \note Should only be called on a client
    DLLEXPORT void HandleWorldFrozenPacket(ResponseWorldFrozen* data);

    //! \brief Applies packets that have been received after the last call to this
    DLLEXPORT void ApplyQueuedPackets();

    //! \todo Expose the parameters and make this activate the fog
    DLLEXPORT void SetFog();
    DLLEXPORT void SetSkyBox(const std::string& materialname);

    //! \brief Alternative to skybox This is a (possibly curved) plane
    //! attached to the camera that can be used to render a background or
    //! a sky
    DLLEXPORT void SetSkyPlane(const std::string& material, const Ogre::Plane& plane
        // = Ogre::Plane(1, 1, 1, 1)
    );

    //! \brief Disables sky plane
    //! \pre SetSkyPlane has been used to set a sky plane
    //! \post The sky plane is disabled
    DLLEXPORT void DisableSkyPlane();


    DLLEXPORT void SetSunlight();
    DLLEXPORT void RemoveSunlight();


    // ------------------------------------ //
    // Script proxies for script system implementation (don't use from c++ systems)

    //! \brief Returns a list of ObjectIDs that have been removed from
    //! any of the specified component types
    DLLEXPORT CScriptArray* GetRemovedIDsForComponents(CScriptArray* componenttypes);

    //! \brief Returns a list of ObjectIDs that have been removed from
    //! any of the script registered component types specified by typeNames
    DLLEXPORT CScriptArray* GetRemovedIDsForScriptComponents(CScriptArray* typenames);

    //! \brief Registers a component type from scripts.
    DLLEXPORT bool RegisterScriptComponentType(
        const std::string& name, asIScriptFunction* factory);

    //! \brief Retrieves a script registered component type holder
    //! \note Increments refcount on return value
    DLLEXPORT ScriptComponentHolder* GetScriptComponentHolder(const std::string& name);

    //! \brief Registers a new system defined in a script. Must
    //! implement the ScriptSystem interface
    DLLEXPORT bool RegisterScriptSystem(const std::string& name, asIScriptObject* system);

    //! \brief Returns the underlying angelscript object that implements a script system
    //! \note Increases refcount on returned object
    DLLEXPORT asIScriptObject* GetScriptSystem(const std::string& name);



    REFERENCE_HANDLE_UNCOUNTED_TYPE(GameWorld);


protected:
    //! \brief Called by Render which is called from a
    //! GraphicalInputEntity if this is linked to one
    DLLEXPORT virtual void RunFrameRenderSystems(int tick, int timeintick);

    //! \brief Called by Tick
    //!
    //! Derived worls should run their systems that need to be ran before the basic systems
    //! and then call this and finally run systems that need to be ran after the base
    //! class' systems (if any)
    DLLEXPORT virtual void _RunTickSystems();

    //! \brief Handles added entities and components
    //!
    //! Construct new nodes based on components values. This is split
    //! into multiple parts to support child classes using the same
    //! Component types in additional nodes
    DLLEXPORT virtual void HandleAddedAndDeleted();

    //! \brief Clears the added components. Call after HandleAddedAndDeleted
    DLLEXPORT virtual void ClearAddedAndRemoved();

    //! \brief Resets stored nodes in systems. Used together with _ResetComponents
    DLLEXPORT virtual void _ResetSystems();

    //! \brief Resets components in holders. Used together with _ResetSystems
    DLLEXPORT virtual void _ResetOrReleaseComponents();

    //! \brief Called in Init when systems should run their initialization logic
    DLLEXPORT virtual void _DoSystemsInit();

    //! \brief Called in Release when systems should run their shutdown logic
    DLLEXPORT virtual void _DoSystemsRelease();

private:
    //! \brief Updates a players position info in this world
    void UpdatePlayersPositionData(ConnectedPlayer& ply);

    void _CreateOgreResources(Ogre::Root* ogre, GraphicalInputEntity* rendertarget);
    void _HandleDelayedDelete();

    //! \brief Reports an entity deletion to clients
    //! \todo Potentially send these in a big blob
    void _ReportEntityDestruction(ObjectID id);

    //! \brief Implementation of doing actual destroy part of removing an entity
    //! \note The caller has to remove the id from Entities
    void _DoDestroy(ObjectID id);

    //! \brief Sends sendable updates to all clients
    void _SendEntityUpdates(ObjectID id, Sendable& sendable, int tick);


    // Packet apply functions //
    void _ApplyInitialEntityPackets();

    void _ApplyEntityUpdatePackets();

protected:
    //! \brief If false a graphical Ogre window hasn't been created
    //! and purely graphical stuff should be skipped
    //!
    //! Used on dedicated servers and other headless applications
    bool GraphicalMode = false;

    //! Bool flag telling whether this is a master world (on a server) or
    //! a mirroring world (client)
    bool IsOnServer = false;

private:
    // pimpl to reduce need of including tons of headers (this causes
    // a double pointer dereference so don't put performance critical
    // stuff here)
    std::unique_ptr<Implementation> pimpl;

    Ogre::Camera* WorldSceneCamera = nullptr;
    Ogre::SceneManager* WorldsScene = nullptr;

    Ogre::CompositorWorkspace* WorldWorkspace = nullptr;

    //! The world is now always linked to a window
    GraphicalInputEntity* LinkedToWindow = nullptr;

    Ogre::Light* Sunlight = nullptr;
    Ogre::SceneNode* SunLightNode = nullptr;

    // physics //
    std::shared_ptr<PhysicalWorld> _PhysicalWorld;

    //! The world can be frozen to stop physics
    bool WorldFrozen = false;


    //! Marks all entities to be released
    bool ClearAllEntities = false;

    //! Holds the players who are receiving this worlds updates and their corresponding
    //! location entities (if any)
    //! \todo Change this to an object that holds more than the player pointer
    std::vector<std::shared_ptr<ConnectedPlayer>> ReceivingPlayers;

    // Entities //
    std::vector<ObjectID> Entities;

    // Parented entities, used to destroy children
    // First is the parent, second is child
    std::vector<std::tuple<ObjectID, ObjectID>> Parents;

    //! The unique ID
    int ID;

    //! The current tick number
    //! This should be the same on all clients as closely as possible
    int TickNumber = 0;

    //! A funky name for this world, if any
    std::string DecoratedName;

    //! If not zero controls the position and properties of WorldSceneCamera
    ObjectID CameraEntity = 0;

    //! The currently applied properties on WorldSceneCamera if the
    //! Camera component of CameraEntity changes (or it is Marked)
    //! these properties are set on WorldSceneCamera
    Camera* AppliedCameraPropertiesPtr = nullptr;


    //! A lock for delayed delete, to allow deleting entities from physical callbacks
    Mutex DeleteMutex;

    //! This vector is used for delayed deletion
    std::vector<ObjectID> DelayedDeleteIDS;

    //! If true any pointers to this world are invalid
    std::shared_ptr<bool> WorldDestroyed = std::make_shared<bool>(false);

    //! Mutex for ConstraintList
    Mutex ConstraintListMutex;

    //! List of constraints in this world
    //!
    //! Used to send full lists to clients
    std::vector<std::shared_ptr<BaseConstraint>> ConstraintList;

    //! Waiting entity packets
    std::vector<std::shared_ptr<NetworkResponse>> InitialEntityPackets;

    //! Waiting update packets
    std::vector<std::shared_ptr<NetworkResponse>> EntityUpdatePackets;
};

} // namespace Leviathan

#ifdef LEAK_INTO_GLOBAL
using Leviathan::GameWorld;
using Leviathan::ObjectID;
#endif
