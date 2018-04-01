// Leviathan Game Engine
// Copyright (c) 2012-2016 Henri Hyyryläinen
#pragma once

//! \file This file contains common components for entities
#include "Define.h"
// ------------------------------------ //
#include "Common/SFMLPackets.h"
#include "Common/Types.h"
#include "Newton/PhysicalWorld.h"
#include "Objects/Constraints.h"

#include "Component.h"
#include "ComponentState.h"

#include "boost/circular_buffer.hpp"

#include <functional>

// This is not optimal to be here but SimpleAnimation would have to
// rehash a string each frame
#include "OgreIdString.h"

namespace Leviathan {

namespace Test {
class TestComponentCreation;
}

//! brief Class containing residue static helper functions
class ComponentHelpers {

    ComponentHelpers() = delete;

    //! \brief Creates a component state from a packet
    static std::shared_ptr<ComponentState> DeSerializeState(sf::Packet& packet);
};

class PositionState;

//! \brief Entity has position and direction it is looking at
//! \note Any possible locking needs to be handled by the caller
//! \todo Initial position states should not be generated or initially sent data shouldn't
//! have the state instead the first state would be guaranteed to be sent after it
class Position : public ComponentWithStates<PositionState> {
public:
    struct Data : public ComponentData {

        Data(const Float3& position, const Float4& orientation) :
            _Position(position), _Orientation(orientation)
        {
        }

        Float3 _Position;
        Float4 _Orientation;
    };

public:
    //! \brief Creates at specific position
    inline Position(const Data& data) : ComponentWithStates(TYPE), Members(data) {}

    REFERENCE_HANDLE_UNCOUNTED_TYPE(Position);

    Data Members;

    static constexpr auto TYPE = COMPONENT_TYPE::Position;
    using StateT = PositionState;
};

//! \brief Entity has an Ogre scene node
//! \note By default this is not marked. If you change Hidden set as marked to
//! update Node state
class RenderNode : public Component {
public:
    DLLEXPORT RenderNode(Ogre::SceneManager* scene);

    //! Test version that doesn't need a valid scene manager
    DLLEXPORT RenderNode(const Test::TestComponentCreation& test);

    //! \brief Gracefully releases while world is still valid
    DLLEXPORT void Release(Ogre::SceneManager* worldsscene);

    REFERENCE_HANDLE_UNCOUNTED_TYPE(RenderNode);

    Ogre::SceneNode* Node = nullptr;

    //! Sets objects attached to the node to be hidden or visible
    bool Hidden = false;

    //! Sets the scale of the node
    Float3 Scale = Float3(1, 1, 1);

    static constexpr auto TYPE = COMPONENT_TYPE::RenderNode;
};

//! \brief Entity is sendable to clients
//! \note This will only be in the entity on the server
class Sendable : public Component {
public:
    //! \note This is not thread safe do not call CheckReceivedPackets and AddSentPacket
    //! at the same time
    //! \todo Make sure that CheckReceivedPackages is called for entities that have
    //! stopped moving ages ago to free up memory
    class ActiveConnection {
    public:
        inline ActiveConnection(std::shared_ptr<Connection> connection) :
            CorrespondingConnection(connection), LastConfirmedTickNumber(-1)
        {
        }

        //! \brief Checks has any packet been successfully received and updates
        //! last confirmed
        DLLEXPORT void CheckReceivedPackets();

        //! \brief Adds a package to be checked for finalization in CheckReceivedPackages
        inline void AddSentPacket(int tick, std::shared_ptr<ComponentState> state,
            std::shared_ptr<SentNetworkThing> packet)
        {
            SentPackets.push_back(std::make_tuple(tick, state, packet));
        }

        std::shared_ptr<Connection> CorrespondingConnection;

        //! Data used to build a delta update packet
        //! \note This is set to be the last known successfully sent state to
        //! avoid having to resend intermediate steps
        std::shared_ptr<ComponentState> LastConfirmedData;

        //! The tick number of the confirmed state
        //! If a state is confirmed as received that has number higher than this
        // LastConfirmedData will be replaced.
        int LastConfirmedTickNumber;

        //! Holds packets sent to this connection that haven't failed or been received yet
        //! \todo Move this into GameWorld to keep a single list of connected players
        std::vector<std::tuple<int, std::shared_ptr<ComponentState>,
            std::shared_ptr<SentNetworkThing>>>
            SentPackets;
    };

public:
    inline Sendable() : Component(TYPE) {}

    inline void AddConnectionToReceivers(std::shared_ptr<Connection> connection)
    {

        UpdateReceivers.push_back(std::make_shared<ActiveConnection>(connection));
    }

    REFERENCE_HANDLE_UNCOUNTED_TYPE(Sendable);

    //! Clients we have already sent a state to
    std::vector<std::shared_ptr<ActiveConnection>> UpdateReceivers;

    static constexpr auto TYPE = COMPONENT_TYPE::Sendable;
};

//! \brief Entity is received from a server
//!
//! Sendable counterpart on the client
class Received : public Component {
public:
    //! \brief Storage class for ObjectDeltaStateData
    //! \todo Possibly add move constructors
    class StoredState {
    public:
        inline StoredState(std::shared_ptr<ComponentState> safedata, int tick, void* data) :
            DeltaData(safedata), Tick(tick), DirectData(data)
        {
        }

        std::shared_ptr<ComponentState> DeltaData;

        //! Tick number, should be retrieved from DeltaData
        int Tick;

        //! This avoids using dynamic_cast
        void* DirectData;
    };

public:
    inline Received() : Component(TYPE), ClientStateBuffer(BASESENDABLE_STORED_RECEIVED_STATES)
    {
    }

    DLLEXPORT void GetServerSentStates(StoredState const** first, StoredState const** second,
        int tick, float& progress) const;

    REFERENCE_HANDLE_UNCOUNTED_TYPE(Received);

    //! Client side buffer of past states
    boost::circular_buffer<StoredState> ClientStateBuffer;

    //! If true this uses local control and will send updates to the server
    bool LocallyControlled = false;

    static constexpr auto TYPE = COMPONENT_TYPE::Received;
};


//! \brief Entity has a box for geometry/model, possibly also physics
class BoxGeometry : public Component {
public:
    inline BoxGeometry(const Float3& size, const std::string& material) :
        Component(TYPE), Sizes(size), Material(material)
    {
    }

    REFERENCE_HANDLE_UNCOUNTED_TYPE(BoxGeometry);

    //! Size along the axises
    Float3 Sizes;

    //! Rendering surface material name
    std::string Material;

    //! Entity created from a box mesh
    Ogre::Item* GraphicalObject = nullptr;

    static constexpr auto TYPE = COMPONENT_TYPE::BoxGeometry;
};

//! \brief Entity has a model
class Model : public Component {
public:
    DLLEXPORT Model(
        Ogre::SceneManager* scene, Ogre::SceneNode* parent, const std::string& meshname);

    //! \brief Destroys GraphicalObject
    DLLEXPORT void Release(Ogre::SceneManager* scene);

    REFERENCE_HANDLE_UNCOUNTED_TYPE(Model);

    //! The entity that has this model's mesh loaded
    Ogre::Item* GraphicalObject = nullptr;

    static constexpr auto TYPE = COMPONENT_TYPE::Model;
};

//! \brief Contains an nimation for Animated component
struct SimpleAnimation {

    DLLEXPORT inline SimpleAnimation(const std::string& name) : Name(name), ReadableName(name)
    {
    }

    DLLEXPORT inline SimpleAnimation(SimpleAnimation&& other) :
        Name(std::move(other.Name)), ReadableName(std::move(other.ReadableName))
    {
        Loop = other.Loop;
        SpeedFactor = other.SpeedFactor;
        Paused = other.Paused;
    }

    DLLEXPORT inline SimpleAnimation(const SimpleAnimation& other) :
        Name(other.Name), ReadableName(other.ReadableName)
    {
        Loop = other.Loop;
        SpeedFactor = other.SpeedFactor;
        Paused = other.Paused;
    }

    const Ogre::IdString Name;

    //! Readable version of Name as it is hashed
    const std::string ReadableName;

    //! If true the animation will automatically loop
    bool Loop = false;
    //! Controls how fast the animation plays
    float SpeedFactor = 1;
    //! If true then the animation isn't updated
    bool Paused = false;
};

//! \brief Entity plays animations on an Ogre::Item
class Animated : public Component {
public:
    DLLEXPORT Animated(Ogre::Item* item) : Component(TYPE), GraphicalObject(item) {}

    REFERENCE_HANDLE_UNCOUNTED_TYPE(Model);

    //! The entity that is played animations on
    Ogre::Item* GraphicalObject = nullptr;

    //! Playing animations
    //! \note When adding or removing (or changing
    //! the looping etc. properties) this needs to be marked
    std::vector<SimpleAnimation> Animations;

    static constexpr auto TYPE = COMPONENT_TYPE::Animated;
};

//! \brief Plane component
//!
//! Creates a static mesh for this
class Plane : public Component {
public:
    DLLEXPORT Plane(Ogre::SceneManager* scene, Ogre::SceneNode* parent,
        const std::string& material, const Ogre::Plane& plane, const Float2& size);

    //! \brief Destroys GraphicalObject
    DLLEXPORT void Release(Ogre::SceneManager* scene);

    REFERENCE_HANDLE_UNCOUNTED_TYPE(Plane);

    //! The plane that this component creates
    Ogre::Item* GraphicalObject = nullptr;

    const std::string GeneratedMeshName;

    static constexpr auto TYPE = COMPONENT_TYPE::Plane;
};


//! \brief Entity has a physical component
//! \pre Entity has Position component
//! \todo Global Newton lock
class Physics : public Component {
public:
    //! \brief Holder for information regarding a single force
    class ApplyForceInfo {
    public:
        //! \note Pass NULL for name if not used, avoid passing empty strings
        //! \param name The name to assign. This will be deleted by a std::unique_ptr
        ApplyForceInfo(bool addmass,
            std::function<Float3(ApplyForceInfo* instance, Physics& object)> getforce,
            std::unique_ptr<std::string> name = nullptr) :
            OptionalName(move(name)),
            MultiplyByMass(addmass), Callback(getforce)
        {
        }

        ApplyForceInfo(const ApplyForceInfo& other) :
            MultiplyByMass(other.MultiplyByMass), Callback(other.Callback)
        {
            if(other.OptionalName)
                OptionalName = std::make_unique<std::string>(*other.OptionalName);
        }

        ApplyForceInfo(ApplyForceInfo&& other) :
            OptionalName(move(other.OptionalName)),
            MultiplyByMass(std::move(other.MultiplyByMass)), Callback(move(other.Callback))
        {
        }

        ApplyForceInfo& operator=(const ApplyForceInfo& other)
        {

            if(other.OptionalName)
                OptionalName = std::make_unique<std::string>(*other.OptionalName);

            MultiplyByMass = other.MultiplyByMass;
            Callback = other.Callback;

            return *this;
        }

        //! Set a name when you don't want other non-named forces to override this
        std::unique_ptr<std::string> OptionalName;

        //! Whether to multiply the force by mass, makes acceleration constant with
        //! different masses
        bool MultiplyByMass;

        //! The callback which returns the force
        //! \todo Allow deleting this force from the callback
        std::function<Float3(ApplyForceInfo* instance, Physics& object)> Callback;
    };

    struct BasePhysicsData {

        Float3 Velocity;
        Float3 Torque;
    };

    struct Data {

        ObjectID id;
        GameWorld* world;
        Position& updatepos;
        Sendable* updatesendable;
    };

public:
    inline Physics(const Data& args) :
        Component(TYPE), World(args.world), _Position(args.updatepos), ThisEntity(args.id),
        UpdateSendable(args.updatesendable)
    {
    }
    DLLEXPORT ~Physics();

    //! \brief Destroys the physical body
    DLLEXPORT void Release();

    //! \brief Sets collision when body hasn't been created yet
    DLLEXPORT bool SetCollision(NewtonCollision* collision);

    //! \brief Use this to create a body for this component once Collision is set
    //! \param physicsmaterialid Retrieve from the same world with
    //! `GameWorld::GetPhysicalMaterial`. -1 to use default material
    DLLEXPORT NewtonBody* CreatePhysicsBody(PhysicalWorld* world, int physicsmaterialid = -1);

    DLLEXPORT void GiveImpulse(const Float3& deltaspeed, const Float3& point = Float3(0));

    //! \brief Adds an apply force
    //! \note Overwrites old forces with the same name
    //! \param pointertohandle Pointer to the force which will be deleted by this
    DLLEXPORT void ApplyForce(ApplyForceInfo* pointertohandle);

    //! \brief Removes an existing ApplyForce
    //! \param name name of force to delete, pass empty std::string to delete the
    //! default named force
    DLLEXPORT bool RemoveApplyForce(const std::string& name);

    //! \brief Add force to the object
    //! \note This is applied in ApplyForceAndTorqueEvent
    DLLEXPORT void AddForce(const Float3& force);

    //! Overrides this objects velocity in ApplyForceAndTorqueEvent
    DLLEXPORT void SetVelocity(const Float3& velocities);

    //! \brief Clears velocity and last frame forces (but not the applied force list)
    DLLEXPORT void ClearVelocity();

    //! \brief Gets the absolute velocity
    DLLEXPORT Float3 GetVelocity() const;

    //! \brief Gets the omega (angular velocity)
    DLLEXPORT Float3 GetOmega() const;

    //! \brief Sets the omega
    DLLEXPORT void SetOmega(const Float3& velocities);

    //! \brief Add to the omega
    DLLEXPORT void AddOmega(const Float3& velocities);

    //! \brief Adds torque to the object
    //! \note This is applied in ApplyForceAndTorqueEvent
    DLLEXPORT void AddTorque(const Float3& torque);

    //! \brief Overrides this object's torque in ApplyForceAndTorqueEvent
    DLLEXPORT void SetTorque(const Float3& torque);

    //! \brief Gets the torque of the body (rotational velocity)
    DLLEXPORT Float3 GetTorque() const;

    //! \brief Sets the physical material ID of this object
    //! \note You have to fetch the ID from the world's corresponding PhysicalMaterialManager
    DLLEXPORT void SetPhysicalMaterialID(int ID);

    //! \brief Sets the linear dampening which slows down the object
    //! \param factor The factor to set. Must be between 0.f and 1.f. Default is 0.1f
    //! \note This can be used to set the viscosity of the substance the object is in
    //! for example to mimic drag in water (this needs verification...)
    //!
    //! More on this in the Newton wiki here:
    //! http://newtondynamics.com/wiki/index.php5?title=NewtonBodySetLinearDamping
    DLLEXPORT void SetLinearDamping(float factor = 0.1f);

    //! \brief Sets the angular damping.
    DLLEXPORT void SetAngularDamping(const Float3& factor = Float3(0.1f));

    //! \brief Applies physical state from holder object
    DLLEXPORT void ApplyPhysicalState(const BasePhysicsData& data);

    //! \brief Syncs this physics body to a changed position.
    //!
    //! Call after making changes to Position component if you don't want this physics
    //! body to overwrite the change on next tick.
    DLLEXPORT void JumpTo(Position& target);

    //! \brief Moves the physical body to the specified position
    //! \returns False if this fails because there currently is no physics body
    //! for this component
    DLLEXPORT bool SetPosition(const Float3& pos, const Float4& orientation);

    //! \brief Same as SetPosition but only sets orientation
    DLLEXPORT bool SetOnlyOrientation(const Float4& orientation);

    //! \brief Returns the full matrix representing this body's position and rotation
    DLLEXPORT Ogre::Matrix4 GetFullMatrix() const;

    //! \brief Calculates the mass matrix and applies the mass parameter to the body
    DLLEXPORT void SetMass(float mass);

    //! \brief Adds a constraint to the current Body to only move in place
    DLLEXPORT bool CreatePlaneConstraint(
        PhysicalWorld* world, const Float3& planenormal = Float3(0, 1, 0));


    // default physics callbacks that are fine in most cases //
    // Don't forget to pass the user data as BaseObject if using these //
    static void ApplyForceAndTorqueEvent(
        const NewtonBody* const body, dFloat timestep, int threadIndex);

    static void DestroyBodyCallback(const NewtonBody* body);

    static void PhysicsMovedEvent(
        const NewtonBody* const body, const dFloat* const matrix, int threadIndex);

    REFERENCE_HANDLE_UNCOUNTED_TYPE(Physics);

    //! \brief Adds all applied forces together
    Float3 _GatherApplyForces(const float& mass);

    DLLEXPORT float GetMass() const
    {
        return Mass;
    }

    inline NewtonBody* GetBody() const
    {
        return Body;
    }

    inline NewtonCollision* GetCollision() const
    {
        return Collision;
    }

private:
    NewtonCollision* Collision = nullptr;
    NewtonBody* Body = nullptr;

    //! The set physical material
    //! If none is set this defaults to -1
    //! The default material ID from GetDefaultPhysicalMaterialID might be applied
    int AppliedPhysicalMaterial = -1;

    //! Non-newton access to mass
    // 0 mass means static
    float Mass = 0.f;

    bool ApplyGravity = true;

    std::list<std::shared_ptr<ApplyForceInfo>> ApplyForceList;

    // Stores velocity and torque that should be set in the
    // callback. These are reset after each apply
    // If the override is true then the value is directly set
    Float3 SumQueuedForce = Float3(0, 0, 0);
    Float3 SumQueuedTorque = Float3(0, 0, 0);
    bool TorqueOverride = false;

public:
    //! Used to access gravity data
    GameWorld* World = nullptr;

    //! Physics object requires a position
    Position& _Position;

    //! For access from physics callbacks
    const ObjectID ThisEntity;

    // Optional access to other components that can be used for marking when physics object
    // moves
    Sendable* UpdateSendable = nullptr;

    static constexpr auto TYPE = COMPONENT_TYPE::Physics;
};

class ManualObject : public Component {
public:
    DLLEXPORT ManualObject(Ogre::SceneManager* scene);

    DLLEXPORT void Release(Ogre::SceneManager* scene);

    REFERENCE_HANDLE_UNCOUNTED_TYPE(ManualObject);

    Ogre::ManualObject* Object = nullptr;

    //! When not empty the ManualObject has been created into an actual mesh
    //! that needs to be destroyed on release
    //! \note The Object can be directly added to a scene so this may be empty even
    //! if the Object is created
    std::string CreatedMesh;

    static constexpr auto TYPE = COMPONENT_TYPE::ManualObject;
};


// class Parent : public Component{
// public:

//     struct Data{

//         std::vector<ObjectID> EntityIDs;
//     };

// public:
//     DLLEXPORT Parent();

//     DLLEXPORT Parent(const Data &data, GameWorld* world, Lock &worldlock);

//     //! \brief Removes child object without notifying it
//     DLLEXPORT void RemoveChildNoNotify(Parentable* which);

//     //! \brief Removes all children notifying them
//     DLLEXPORT void RemoveChildren();

//     DLLEXPORT void AddDataToPacket(sf::Packet &packet) const;

//     //! \note The packet needs to be checked that it is still valid after this call
//     DLLEXPORT static Data LoadDataFromPacket(sf::Packet &packet);

//     //! \brief Does everything necessary to attach a child
//     DLLEXPORT void AddChild(ObjectID childid, Parentable &child);

//     //! Attached children which can be moved at certain times
//     //! \todo Make improvements to component lookup performance through this
//     std::vector<std::tuple<ObjectID, Parentable*>> Children;
// };



// class Constraintable : public Component{
// public:
//     //! \param world Used to allow created constraints to access world data (including
//     physics) DLLEXPORT Constraintable(ObjectID id, GameWorld* world);

//     //! \brief Destroys all constraints attached to this
//     DLLEXPORT ~Constraintable();

//     //! Creates a constraint between this and another object
//     //! \warning DO NOT store the returned value (since that reference isn't counted)
//     //! \note Before the constraint is actually finished, you need to
//     //! call ->SetParameters() on the returned ptr
//     //! and then ->Init() and then let go of the ptr
//     //! \note If you do not want to allow constraints where child is NULL you have to
//     //! check if child is NULL before calling this function
//     template<class ConstraintClass, typename... Args>
//     std::shared_ptr<ConstraintClass> CreateConstraintWith(Constraintable &other,
//         Args&&... args)
//     {
//         auto tmpconstraint = std::make_shared<ConstraintClass>(World, *this, other,
//         args...);

//         if(tmpconstraint)
//             _NotifyCreate(tmpconstraint, other);

//         return tmpconstraint;
//     }

//     DLLEXPORT void RemoveConstraint(BaseConstraint* removed);

//     DLLEXPORT void AddConstraint(std::shared_ptr<BaseConstraint> added);

// protected:

//     //! \brief Notifies the other object and the GameWorld of the new constraint
//     DLLEXPORT void _NotifyCreate(std::shared_ptr<BaseConstraint> newconstraint,
//         Constraintable &other);

// public:


//     std::vector<std::shared_ptr<BaseConstraint>> PartInConstraints;

//     GameWorld* World;

//     //! ID for other component lookup
//     ObjectID PartOfEntity;
// };

// class Trail : public Component{
// public:

//     struct ElementProperties{
//         ElementProperties(const Float4 &initialcolour,
//             const Float4 &colourchange, const float &initialsize, const float &sizechange) :
//             InitialColour(initialcolour), ColourChange(colourchange),
//             InitialSize(initialsize), SizeChange(sizechange)
//         {

//         }

//         ElementProperties(const Float4 &initialcolour,
//             const float &initialsize) :
//             InitialColour(initialcolour), ColourChange(0), InitialSize(initialsize),
//             SizeChange(0)
//         {

//         }

//         ElementProperties() :
//             InitialColour(1), ColourChange(0), InitialSize(1), SizeChange(0)
//         {

//         }

//         Float4 InitialColour;
//         Float4 ColourChange;
//         float InitialSize;
//         float SizeChange;
//     };

//     struct Properties{
//     public:
//         Properties(size_t maxelements, float lenght, float maxdistance,
//             bool castshadows = false) :
//             TrailLenght(lenght), MaxDistance(maxdistance),
//             MaxChainElements(maxelements), CastShadows(castshadows), Elements(1)
//         {
//         }

//         float TrailLenght;
//         float MaxDistance;
//         size_t MaxChainElements;
//         bool CastShadows;

//         std::vector<ElementProperties> Elements;
//     };


// public:

//     DLLEXPORT Trail(RenderNode* node, const std::string &materialname,
//         const Properties &variables);

//     //! \brief Sets properties on the trail object
//     //! \pre Ogre objects have been created
//     //! \param force If set to true all settings will be applied
// 	DLLEXPORT bool SetTrailProperties(const Properties &variables, bool force = false);

//     //! \brief Destroys the TrailEntity
//     DLLEXPORT void Release(Ogre::SceneManager* scene);

//     //! The trail entity which is attached at the root scene node and follows our RenderNode
//     //! component around
// 	Ogre::RibbonTrail* TrailEntity = nullptr;

//     //! For ease of use direct access to ogre node is required
//     //! Not valid in non-gui mode
//     RenderNode* _RenderNode = nullptr;

//     //! The used trail material
//     std::string Material;

//     //! Current settings, when updating settings only changed ones are applied
//     Properties CurrentSettings;
// };

// //! \todo Add the Markers to the actual world for sending over the network individually
// class PositionMarkerOwner : public Component{
// public:

//     struct Data{

//         std::vector<std::tuple<ObjectID, Float3, Float4>> EntityPositions;
//     };

// public:
//     DLLEXPORT PositionMarkerOwner();

//     //! \brief Create with automatically created positions
//     DLLEXPORT PositionMarkerOwner(const Data &positions, GameWorld* world,
//         Lock &worldlock);

//     //! \brief Queues destruction and clears the list of Markers
//     DLLEXPORT void Release(GameWorld* world, Lock &worldlock);

//     //! Adds a node
//     //! \todo Allow not deleting entities on release
//     DLLEXPORT void Add(ObjectID entity, Position& pos);

//     DLLEXPORT void AddDataToPacket(sf::Packet &packet) const;

//     //! \note The packet needs to be checked that it is valid after this call
//     DLLEXPORT static Data LoadDataFromPacket(sf::Packet &packet);

//     std::vector<std::tuple<ObjectID, Position*>> Markers;
// };


//! \brief Properties that a camera entity has (will also need a Position component)
class Camera : public Component {
public:
    //! \brief Creates at specific position
    inline Camera(uint8_t fovy = 60, bool soundperceiver = true) :
        Component(TYPE), FOVY(fovy), SoundPerceiver(soundperceiver)
    {
    }

    REFERENCE_HANDLE_UNCOUNTED_TYPE(Camera);

    //! Y-axis based field of view.
    //! \warning This is different than the usual x-axis based field of view!
    //! See the Ogre manual for details: Ogre::Frustum::setFOVy (const Radian & fovy )
    //!
    //! Normal range is 45 to 60
    uint8_t FOVY;
    bool SoundPerceiver;
    // TODO: orthographic
    // bool Orthographic;

    static constexpr auto TYPE = COMPONENT_TYPE::Camera;
};

} // namespace Leviathan

#ifdef LEAK_INTO_GLOBAL
using ApplyForceInfo = Leviathan::Physics::ApplyForceInfo;
#endif
