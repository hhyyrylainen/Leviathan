// Leviathan Game Engine
// Copyright (c) 2012-2019 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "Common/ReferenceCounted.h"
#include "Common/ThreadSafe.h"
#include "Component.h"
#include "Networking/CommonNetwork.h"
#include "WorldNetworkSettings.h"

// #include <type_traits>
#include "bsfCore/BsCorePrerequisites.h"

class CScriptArray;
class asIScriptObject;
class asIScriptFunction;


namespace Leviathan {

class Camera;
class PhysicalWorld;
class ScriptComponentHolder;
class ResponseEntityCreation;
class ResponseEntityDestruction;
class ResponseEntityUpdate;
class ResponseEntityLocalControlStatus;

template<class StateT>
class StateHolder;

class EntityState;

struct ComponentTypeInfo {

    inline ComponentTypeInfo(uint16_t ltype, int astype) :
        LeviathanType(ltype), AngelScriptType(astype)
    {}

    uint16_t LeviathanType;
    int AngelScriptType;
};


#define WORLD_CLOCK_SYNC_PACKETS 12
#define WORLD_CLOCK_SYNC_ALLOW_FAILS 2
#define WORLD_OBJECT_UPDATE_CLIENTS_INTERVAL 2

// //! Holds the returned entity that was hit during ray casting
// //! \todo Move to a new file
// class RayCastHitEntity : public ReferenceCounted {
// public:
//     DLLEXPORT RayCastHitEntity(const NewtonBody* ptr = nullptr, const float& tvar = 0.f,
//         RayCastData* ownerptr = nullptr);

//     DLLEXPORT RayCastHitEntity& operator=(const RayCastHitEntity& other);

//     // Compares the hit entity with nullptr //
//     DLLEXPORT bool HasHit();

//     DLLEXPORT Float3 GetPosition();

//     DLLEXPORT bool DoesBodyMatchThisHit(NewtonBody* other);

//     //! Stores the entity, typed as NewtonBody to make sure that user knows
//     //! what should be compared with this
//     const NewtonBody* HitEntity;

//     //! The distance from the start of the ray to the hit location
//     float HitVariable;
//     Float3 HitLocation;
// };

// // Internal object in ray casts //
// struct RayCastData {
//     DLLEXPORT RayCastData(int maxcount, const Float3& from, const Float3& to);
//     DLLEXPORT ~RayCastData();

//     // All hit entities that pass checks //
//     std::vector<RayCastHitEntity*> HitEntities;
//     // Used to stop after certain amount of entities found //
//     int MaxCount;
//     // Used to efficiently calculate many hit locations //
//     Float3 BaseHitLocationCalcVar;
// };

//! \brief Represents a world that contains entities
//!
//! This is the base class from which worlds that support different components are derived from
//! Custom worls should derive from StandardWorld which has all of the standard components
//! supported. See the GenerateStandardWorld.rb file to figure out how to generate world
//! classes
class GameWorld {
    class Implementation;

public:
    //! \param worldid If >= then uses the specified ID for this world. Otherwise one is
    //! automatically generated from IDFactory
    DLLEXPORT GameWorld(int32_t worldtype,
        const std::shared_ptr<PhysicsMaterialManager>& physicsMaterials, int worldid = -1);
    DLLEXPORT ~GameWorld();

    //! \brief Creates resources for the world to work
    //! \post The world can be used after this
    DLLEXPORT bool Init(const WorldNetworkSettings& network, Graphics* graphics);

    //! Release resources
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

    //! \brief Returns the created entity id vector
    DLLEXPORT inline const auto& GetEntities() const
    {
        return Entities;
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
    DLLEXPORT inline int GetTickNumber() const
    {
        return TickNumber;
    }

    //! \brief Returns float between 0.f and 1.f based on how far current tick has progressed
    DLLEXPORT float GetTickProgress() const;

    //! \brief Returns a tuple of the current tick number and how long
    //! it has passed since last tick
    //!
    //! The tick number is always adjusted so that the time since last tick is < TICKSPEED
    DLLEXPORT std::tuple<int, int> GetTickAndTime() const;


    //! \brief Fetches the physical material ID from the material manager
    //! \returns -1 if not found. The id otherwise
    DLLEXPORT int GetPhysicalMaterial(const std::string& name);


    // //! \brief Casts a ray from point along a vector and returns the first physical
    // //! object it hits
    // //! \warning You need to call Release on the returned object once done
    // DLLEXPORT RayCastHitEntity* CastRayGetFirstHit(const Float3& from, const Float3& to);

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

    //! \brief Returns true if entity exists
    DLLEXPORT bool DoesEntityExist(ObjectID id) const
    {
        for(const auto& entity : Entities) {
            if(id == entity)
                return true;
        }

        return false;
    }


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

    //! Helper for getting component of type. This is much slower than
    //! direct lookups with the actual implementation class' GetComponent_Position etc.
    //! methods
    //! \returns null if not found
    template<class TComponent>
    TComponent* GetComponentPtr(ObjectID id)
    {
        std::tuple<void*, bool> component = GetComponent(id, TComponent::TYPE);

        void* ptr = std::get<0>(component);

        if(!ptr)
            return nullptr;

        return static_cast<TComponent*>(ptr);
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

    //! \brief Captures the current state of an entity
    DLLEXPORT virtual void CaptureEntityState(ObjectID id, EntityState& curstate) const;

    //! \brief Captures the initial parameters of an entity with components that don't have
    //! current state synchronization and also types of all components
    //! \returns The count of component data entries in receiver
    DLLEXPORT virtual uint32_t CaptureEntityStaticState(
        ObjectID id, sf::Packet& receiver) const;

    //! \brief Sets the entity that acts as a camera.
    //!
    //! The entity needs at least Position and Camera components
    //! \exception InvalidArgument if the object is missing required components
    DLLEXPORT void SetCamera(ObjectID object);

    //! \brief Casts a ray from the active camera
    //! \param x Pixel x coordinate (range [0, window width])
    //! \param y Pixel y coordinate (range [0, window height])
    //! \exception InvalidState if this world has no active camera
    //! \see SetCamera
    //! \version This now takes in pixel values
    DLLEXPORT bs::Ray CastRayFromCamera(int x, int y) const;

    inline bs::Scene* GetScene()
    {
        return &BSFLayerHack;
        // return WorldsScene;
    }

    //! \brief Returns the scene object the camera is attached to
    //!
    //! Useful to attach backgrounds and other static items to the camera
    DLLEXPORT bs::HSceneObject GetCameraSceneObject();

    // physics functions //
    // DLLEXPORT Float3 GetGravityAtPosition(const Float3& pos);

    inline PhysicalWorld* GetPhysicalWorld()
    {
        return _PhysicalWorld.get();
    }

    //! \returns the unique ID of this world
    DLLEXPORT inline int GetID() const
    {
        return ID;
    }

    //! \returns the type of this world
    DLLEXPORT inline int32_t GetType() const
    {
        return WorldType;
    }

    DLLEXPORT inline const auto& GetNetworkSettings() const
    {
        return NetworkSettings;
    }

    //! \todo Synchronize this over the network
    DLLEXPORT void SetWorldPhysicsFrozenState(bool frozen);

    // Script proxies //

    //
    // Networking methods
    //

    //! \brief Returns true when the player matching the connection should receive updates
    //! about an entity
    //! \todo Implement this
    DLLEXPORT bool ShouldPlayerReceiveEntity(Position& atposition, Connection& connection);

    //! \brief Returns true if a player with the given connection is receiving updates for
    //! this world
    DLLEXPORT bool IsConnectionInWorld(Connection& connection) const;

    //! \brief Verifies that player is receiving this world
    DLLEXPORT void SetPlayerReceiveWorld(std::shared_ptr<ConnectedPlayer> ply);

    //! \brief This is used by Sendable system to loop all players
    //! \returns The list of players in this world
    inline const auto& GetConnectedPlayers() const
    {
        return ReceivingPlayers;
    }

    //! \brief Sends a packet to all connected players
    DLLEXPORT void SendToAllPlayers(
        const std::shared_ptr<NetworkResponse>& response, RECEIVE_GUARANTEE guarantee) const;

    //! \brief Sets local control on a client over an entity or disables it
    //!
    //! \note Only one player can have local control over a single entity at once
    DLLEXPORT void SetLocalControl(
        ObjectID id, bool enabled, const std::shared_ptr<Connection>& allowedconnection);

    //! \returns The list of entities that this client has control over
    DLLEXPORT const auto& GetOurLocalControl() const
    {
        return OurActiveLocalControl;
    }

    //! \returns True if entity is under our local control
    //! \note It doesn't make sense to call this on a server. So if this is needed to be called
    //! where a server might call this you should use "GetNetworkSettings().IsAuthoritative"
    //! instead
    DLLEXPORT inline bool IsUnderOurLocalControl(ObjectID id)
    {
        for(const auto& entity : OurActiveLocalControl)
            if(entity == id)
                return true;
        return false;
    }

    //! \brief Sets a connection that will be used to send local control entity updates to the
    //! server
    DLLEXPORT void SetServerForLocalControl(const std::shared_ptr<Connection>& connection)
    {
        ClientToServerConnection = connection;
    }

    DLLEXPORT const auto& GetServerForLocalControl() const
    {
        return ClientToServerConnection;
    }

    //! \brief Applies an entity update packet
    //! \note With updates the message is queued (and moved) if we don't have the entity
    //! specified by the id
    //! \todo If we receive an update after an entity is destroyed the message should not be
    //! queued
    DLLEXPORT void HandleEntityPacket(ResponseEntityUpdate&& message, Connection& connection);

    //! \returns The id of the created entity or NULL_OBJECT
    DLLEXPORT ObjectID HandleEntityPacket(ResponseEntityCreation& message);

    DLLEXPORT void HandleEntityPacket(ResponseEntityDestruction& message);

    DLLEXPORT void HandleEntityPacket(ResponseEntityLocalControlStatus& message);

    // //! \brief Handles a world clock synchronizing packet
    // //! \note This should only be allowed to be called on a client that has connected
    // //! to a server
    // DLLEXPORT void HandleClockSyncPacket(RequestWorldClockSync* data);

    // //! \brief Handles a world freeze/unfreeze packet
    // //! \note Should only be called on a client
    // DLLEXPORT void HandleWorldFrozenPacket(ResponseWorldFrozen* data);

    DLLEXPORT void SetSunlight();
    DLLEXPORT void RemoveSunlight();
    //! \brief Sets the sunlight properties
    //! \pre SetSunlight has been called
    DLLEXPORT void SetLightProperties(const Float3& colour, float intensity = 0.0001f,
        const Float3& direction = Float3(0.55f, -0.3f, 0.75f), float sourceradius = 0.5f,
        bool castsshadows = true);

    //! \param skyboxname Name of the skybox to set. Or empty to clear the skybox
    DLLEXPORT void SetSkybox(const std::string& skyboxname, float brightness = 1.f);

    //! \brief Sets the world camera eye adaptation settings
    DLLEXPORT void SetAutoExposure(float mineyeadaptation = 0.003f,
        float maxeyeadaptation = 2.0f, float eyeadaptationspeeddown = 3.0f,
        float eyeadaptationspeedup = 3.0f, float histogramlog2max = 4.0f,
        float histogramlog2min = -8.0f, float histogrampcthigh = 0.985f,
        float histogrampctlow = 0.8f);

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

    //! \brief Unregisters a script system that was registered with RegisterScriptSystem
    //! \returns True if unregistered. False if no system with the name was registered
    //! \note This may not be called during a tick while script systems are running (because
    //! this modifies the container for them)
    DLLEXPORT bool UnregisterScriptSystem(const std::string& name);

    //! \brief Returns the underlying angelscript object that implements a script system
    //! \note Increases refcount on returned object
    DLLEXPORT asIScriptObject* GetScriptSystem(const std::string& name);

    // ------------------------------------ //
    // Background worlds (used to stop ticking etc.)

    //! \brief Used to detect that this world is in the background and should not tick
    //! \note Removes the workspace created in OnLinkToWindow
    DLLEXPORT virtual void OnUnLinkedFromWindow(Window* window, Graphics* graphics);

    //! \brief Called when this is added to a Window
    //! \note This creates a compositor workspace that renders this world's scene to the window
    DLLEXPORT virtual void OnLinkToWindow(Window* window, Graphics* graphics);

    //! \brief Configures this world to run tick even when not attached to a window
    DLLEXPORT virtual void SetRunInBackground(bool tickinbackground);


    REFERENCE_HANDLE_UNCOUNTED_TYPE(GameWorld);

protected:
    //! \brief Applies packets that were received out of order. And throws out any too old
    //! packets
    DLLEXPORT void ApplyQueuedPackets();

public:
    //! \brief Called by Render which is called from a
    //! Window if this is linked to one
    //! \protected
    //! \note This is public to allow tests to test interpolation. It might be worth it to make
    //! a separate function that is similar to Render but can be called even in non-gui mode
    DLLEXPORT virtual void RunFrameRenderSystems(int tick, int timeintick);

protected:
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

    //! \brief Called when this is put in the background and systems (the sound system) should
    //! suspend their active objects
    //! \note Only called if TickWhileInBackground is false (and won't be called if that is
    //! changed while this is in the background)
    DLLEXPORT virtual void _DoSuspendSystems();

    //! \brief Opposite of _DoSuspendSystems
    DLLEXPORT virtual void _DoResumeSystems();

    //! \brief Called when this base class wants to create a Sendable component
    DLLEXPORT virtual void _CreateSendableComponentForEntity(ObjectID id);

    //! \brief Called when this base class wants to create a Received component
    DLLEXPORT virtual void _CreateReceivedComponentForEntity(ObjectID id);

    //! \brief Called to deserialize initial entity components and their static state
    //!
    //! This is the reverse operation for CaptureEntityStaticState
    //! \param decodedtype This is used to move unrecognized types up to the base class. -1 if
    //! not fetched yet
    DLLEXPORT virtual void _CreateComponentsFromCreationMessage(
        ObjectID id, sf::Packet& data, int entriesleft, int decodedtype);

    //! \brief Called to deserialize entity component states from a packet
    //! \param decodedtype This is used to move unrecognized types up to the base class. -1 if
    //! not fetched yet
    DLLEXPORT virtual void _CreateStatesFromUpdateMessage(ObjectID id, int32_t ticknumber,
        sf::Packet& data, int32_t referencetick, int decodedtype);

    //! \brief Called to apply local control from a clients message
    //!
    //! Before this is called it is already verified that the update is allowed, but this
    //! method should have cheat checking if that is required
    //! \param decodedtype This is used to move unrecognized types up to the base class. -1 if
    //! not fetched yet
    DLLEXPORT virtual void _ApplyLocalControlUpdateMessage(ObjectID id, int32_t ticknumber,
        sf::Packet& data, int32_t referencetick, int decodedtype);


    //! \brief This method is for doing checks after applying client sent state to the server
    //!
    //! For example to block cheating and reset things on the server. Right now the base
    //! implementation resets the physics position of a moved entity
    DLLEXPORT virtual void _OnLocalControlUpdatedEntity(ObjectID id, int32_t ticknumber);

private:
    //! \brief Updates a players position info in this world
    void UpdatePlayersPositionData(ConnectedPlayer& ply);

    void _CreateRenderingResources(Graphics* graphics);
    void _DestroyRenderingResources();

    void _HandleDelayedDelete();

    //! \brief Reports an entity deletion to clients
    void _ReportEntityDestruction(ObjectID id);

    //! \brief Implementation of doing actual destroy part of removing an entity
    //! \note The caller has to remove the id from Entities
    void _DoDestroy(ObjectID id);

    //! \brief Sends sendable updates to all clients
    void _SendEntityUpdates(ObjectID id, Sendable& sendable, int tick);


protected:
    //! \brief If false a graphical Ogre window hasn't been created
    //! and purely graphical stuff should be skipped
    //!
    //! Used on dedicated servers and other headless applications
    bool GraphicalMode = false;

private:
    // pimpl to reduce need of including tons of headers (this causes
    // a double pointer dereference so don't put performance critical
    // stuff here)
    std::unique_ptr<Implementation> pimpl;

    //! A temporary solution around no multiple scenes in BSF
    bs::Scene BSFLayerHack = 0;

    // Ogre::Camera* WorldSceneCamera = nullptr;
    // Ogre::SceneManager* WorldsScene = nullptr;

    // Ogre::CompositorWorkspace* WorldWorkspace = nullptr;

    //! The world is now always linked to a window
    Window* LinkedToWindow = nullptr;

    // Ogre::Light* Sunlight = nullptr;
    // Ogre::SceneNode* SunLightNode = nullptr;

    // physics //
    std::shared_ptr<PhysicsMaterialManager> PhysicsMaterials;
    std::shared_ptr<PhysicalWorld> _PhysicalWorld;

    //! The world can be frozen to stop physics
    bool WorldFrozen = false;


    //! Marks all entities to be released
    bool ClearAllEntities = false;

    //! Holds the players who are receiving this worlds updates and their corresponding
    //! location entities (if any)
    //! \todo Change this to an object that holds more than the player pointer
    std::vector<std::shared_ptr<ConnectedPlayer>> ReceivingPlayers;

    //! Active local control entities on clients (this variant is on the Server,
    //! OurActiveLocalControl is on the client)
    std::map<ObjectID, Connection*> ActiveLocalControl;

    //! Connection from client world to server world. Used to send local control updates
    std::shared_ptr<Connection> ClientToServerConnection;

    //! Active local controls for this client world
    std::vector<ObjectID> OurActiveLocalControl;

    //! Primary network settings for controlling what state synchronization methods are called
    WorldNetworkSettings NetworkSettings;

    // //! List of newly created entities that need to be created (or they have added new
    // //! components and the initial value needs to be sent again)
    // std::vector<ObjectID> NewlyCreatedEntities;

    // Entities //
    std::vector<ObjectID> Entities;

    // Parented entities, used to destroy children
    // First is the parent, second is child
    std::vector<std::tuple<ObjectID, ObjectID>> Parents;

    //! The unique ID
    const int ID;

    //! The world type. This is needed for telling clients which world objects to create when
    //! they connect
    const int32_t WorldType;

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

    //! True while in a tick. Used to prevent destroying entities or components
    //! \todo This check needs to be added to component removal
    bool TickInProgress = false;

    //! If true this will keep running while not attached to a window
    bool TickWhileInBackground = false;

    //! Set by OnLinkToWindow when this is added to a Window
    //! \note This must be added to the same one that Init was called with
    //! \todo Determine if worlds could be linked to a different Window than the
    //! one it was created with
    //! \see Window::LinkObjects
    bool InBackground = true;

    //! A lock for delayed delete, to allow deleting entities from physical callbacks
    Mutex DeleteMutex;

    //! This vector is used for delayed deletion
    std::vector<ObjectID> DelayedDeleteIDS;

    // //! If true any pointers to this world are invalid
    // std::shared_ptr<bool> WorldDestroyed = std::make_shared<bool>(false);
};

} // namespace Leviathan

#ifdef LEAK_INTO_GLOBAL
using Leviathan::GameWorld;
using Leviathan::ObjectID;
#endif
