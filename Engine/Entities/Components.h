// Leviathan Game Engine
// Copyright (c) 2012-2019 Henri Hyyryläinen
#pragma once

//! \file This file contains common components for entities
#include "Define.h"
// ------------------------------------ //
#include "Common/SFMLPackets.h"
#include "Common/Types.h"
#include "Physics/PhysicsBody.h"
#include "Physics/PhysicsShape.h"

#include "Component.h"
#include "ComponentState.h"

#include "bsfCore/BsCorePrerequisites.h"

#include <functional>

namespace Leviathan {

class PhysicalWorld;

namespace Test {
class TestComponentCreation;
}

class PositionState;

//! \brief Entity has position and direction it is looking at
//! \note Any possible locking needs to be handled by the caller
//! \todo Initial position states should not be generated or initially sent data shouldn't
//! have the state instead the first state would be guaranteed to be sent after it
class Position : public ComponentWithStates<PositionState>, public PhysicsPositionProvider {
public:
    struct Data {

        Data(const Float3& position, const Float4& orientation) :
            _Position(position), _Orientation(orientation)
        {}

        Float3 _Position;
        Float4 _Orientation;
    };

public:
    //! \brief Creates at specific position
    inline Position(const Data& data) : ComponentWithStates(TYPE), Members(data) {}

    void GetPositionDataForPhysics(
        const Float3*& position, const Float4*& orientation) const override
    {
        position = &Members._Position;
        orientation = &Members._Orientation;
    }

    void SetPositionDataFromPhysics(const Float3& position, const Float4& orientation) override
    {
        Members._Position = position;
        Members._Orientation = orientation;
        Marked = true;
    }

    //! \brief Applies the state and marks this as changed. Doesn't check if the state is
    //! actually different
    DLLEXPORT void ApplyState(const PositionState& state);

    REFERENCE_HANDLE_UNCOUNTED_TYPE(Position);

    Data Members;

    static constexpr auto TYPE = COMPONENT_TYPE::Position;
    using StateT = PositionState;
};

//! \brief Entity has a scene node
//! \note By default this is not marked. If you change Hidden set as marked to
//! update Node state
class RenderNode : public Component {
public:
    DLLEXPORT RenderNode(bs::Scene* scene);

    //! Test version that doesn't need a valid scene manager
    DLLEXPORT RenderNode(const Test::TestComponentCreation& test);

    //! \brief Gracefully releases while world is still valid
    DLLEXPORT void Release(bs::Scene* worldsscene);

    REFERENCE_HANDLE_UNCOUNTED_TYPE(RenderNode);

    bs::HSceneObject Node;

    //! Sets objects attached to the node to be hidden or visible
    bool Hidden = false;

    //! Sets the scale of the node
    Float3 Scale = Float3(1, 1, 1);

    // TODO: implement
    //! Attaches this node to this parent (if null entity then attached to the scene root)
    // ObjectID ParentEntity = NULL_OBJECT;

    //! Don't touch
    bs::Scene Scene;

    static constexpr auto TYPE = COMPONENT_TYPE::RenderNode;
};

//! \brief Entity is sendable to clients
//! \note This will only be in the entity on the server
//!
//! It can get pretty expensive to remove closed connections from all Sendable objects
//! \todo The world needs to clean ActiveConnection objects from all Sendables related to a
//! closed connection
class Sendable : public Component {
public:
    //! \note This is not thread safe do not call CheckReceivedPackets and AddSentPacket
    //! at the same time
    //! \todo Make sure that CheckReceivedPackages is called for entities that have
    //! stopped moving ages ago to free up memory
    class ActiveConnection {
    public:
        inline ActiveConnection(const std::shared_ptr<Connection>& connection) :
            CorrespondingConnection(connection)
        {}

        //! \brief Checks has any packet been successfully received and updates
        //! last confirmed
        DLLEXPORT void CheckReceivedPackets();

        //! \brief Adds a package to be checked for finalization in CheckReceivedPackages
        inline void AddSentPacket(int tick, const std::shared_ptr<EntityState>& state,
            std::shared_ptr<SentNetworkThing> packet)
        {
            SentPackets.emplace_back(tick, state, packet);
        }

        std::shared_ptr<Connection> CorrespondingConnection;

        //! Data used to build a delta update packet
        //! \note This is set to be the last known successfully sent state to
        //! avoid having to resend intermediate steps
        std::shared_ptr<EntityState> LastConfirmedData;

        //! The tick number of the confirmed state
        //! If a state is confirmed as received that has number higher than this
        // LastConfirmedData will be replaced.
        int LastConfirmedTickNumber = -1;

        //! Holds packets sent to this connection that haven't failed or been received yet
        std::vector<
            std::tuple<int, std::shared_ptr<EntityState>, std::shared_ptr<SentNetworkThing>>>
            SentPackets;
    };

public:
    inline Sendable() : Component(TYPE) {}

    REFERENCE_HANDLE_UNCOUNTED_TYPE(Sendable);

    //! Clients we have already sent a state to
    std::vector<ActiveConnection> UpdateReceivers;

    static constexpr auto TYPE = COMPONENT_TYPE::Sendable;
};

//! \brief Entity is received from a server
//!
//! Sendable counterpart on the client
class Received : public Component {
public:
    // //! \brief Storage class for ObjectDeltaStateData
    // //! \todo Possibly add move constructors
    // class StoredState {
    // public:
    //     inline StoredState(std::shared_ptr<ComponentState> safedata, int tick, void* data) :
    //         DeltaData(safedata), Tick(tick), DirectData(data)
    //     {}

    //     std::shared_ptr<ComponentState> DeltaData;

    //     //! Tick number, should be retrieved from DeltaData
    //     int Tick;

    //     //! This avoids using dynamic_cast
    //     void* DirectData;
    // };

public:
    inline Received() :
        Component(TYPE) //, ClientStateBuffer(BASESENDABLE_STORED_RECEIVED_STATES)
    {}

    // DLLEXPORT void GetServerSentStates(StoredState const** first, StoredState const**
    // second,
    //     int tick, float& progress) const;

    REFERENCE_HANDLE_UNCOUNTED_TYPE(Received);

    // //! Client side buffer of past states
    // boost::circular_buffer<StoredState> ClientStateBuffer;

    //! If true this uses local control and will send updates to the server
    bool LocallyControlled = false;

    static constexpr auto TYPE = COMPONENT_TYPE::Received;
};


//! \brief Entity has a box for geometry/model, possibly also physics
class BoxGeometry : public Component {
public:
    inline BoxGeometry(const Float3& size, const std::string& material) :
        Component(TYPE), Sizes(size), Material(material)
    {}

    REFERENCE_HANDLE_UNCOUNTED_TYPE(BoxGeometry);

    //! Size along the axises
    Float3 Sizes;

    //! Rendering surface material name
    std::string Material;

    // //! Entity created from a box mesh
    // Ogre::Item* GraphicalObject = nullptr;

    static constexpr auto TYPE = COMPONENT_TYPE::BoxGeometry;
};

//! \brief Entity has a model
class Model : public Component {
public:
    DLLEXPORT Model(bs::Scene* scene, RenderNode& parent, const std::string& meshname,
        const bs::HMaterial& material);

    DLLEXPORT void Release();

    DLLEXPORT void ApplyMeshName();

    REFERENCE_HANDLE_UNCOUNTED_TYPE(Model);

    //! The entity that has this model's mesh loaded
    bs::HRenderable GraphicalObject;

    //! \note Changing this currently does nothing
    std::string MeshName;

    //! Material set on the object
    bs::HMaterial Material;

    static constexpr auto TYPE = COMPONENT_TYPE::Model;
};

//! \brief Contains an animation for Animated component
struct SimpleAnimation {
    friend class AnimationSystem;

public:
    DLLEXPORT inline SimpleAnimation(const std::string& name) : Name(name) {}

    DLLEXPORT inline SimpleAnimation(SimpleAnimation&& other) :
        Name(std::move(other.Name)), Loop(other.Loop), SpeedFactor(other.SpeedFactor),
        Paused(other.Paused), _LoadedAnimation(std::move(other._LoadedAnimation))
    {
        Loop = other.Loop;
        SpeedFactor = other.SpeedFactor;
        Paused = other.Paused;
    }

    DLLEXPORT inline SimpleAnimation(const SimpleAnimation& other) :
        Name(other.Name), Loop(other.Loop), SpeedFactor(other.SpeedFactor),
        Paused(other.Paused), _LoadedAnimation(other._LoadedAnimation)
    {}

    //! \note Updating this only takes effect if NameMarked is set to true
    std::string Name;
    bool NameMarked = true;

    //! If true the animation will automatically loop
    bool Loop = false;

    //! Controls how fast the animation plays
    float SpeedFactor = 1;

    //! If true then the animation will be paused
    bool Paused = false;

protected:
    //! Loaded animation
    bs::HAnimationClip _LoadedAnimation;
};

//! \brief Entity plays animations on an Ogre::Item
class Animated : public Component {
public:
    DLLEXPORT Animated(RenderNode& node);

    DLLEXPORT void Release();

    REFERENCE_HANDLE_UNCOUNTED_TYPE(Animated);

    //! Created animation component for this entity
    bs::HAnimation Animation;

    //! Playing animations
    //! \note When adding or removing (or changing
    //! the looping etc. properties) this needs to be marked
    std::vector<SimpleAnimation> Animations;

    static constexpr auto TYPE = COMPONENT_TYPE::Animated;
};

// //! \brief Plane component
// //!
// //! Creates a static mesh for this
// class Plane : public Component {
// public:
//     DLLEXPORT Plane(bs::Scene* scene, Ogre::SceneNode* parent,
//         const std::string& material, const Ogre::Plane& plane, const Float2& size,
//         const Ogre::Vector3& uvupvector = Ogre::Vector3::UNIT_Y);

//     //! \brief Destroys GraphicalObject
//     DLLEXPORT void Release(bs::Scene* scene);

//     REFERENCE_HANDLE_UNCOUNTED_TYPE(Plane);

//     //! The plane that this component creates
//     Ogre::Item* GraphicalObject = nullptr;

//     const std::string GeneratedMeshName;

//     // Changing any of these does nothing
//     std::string Material;
//     Ogre::Plane PlaneDefinition;
//     Float2 Size;
//     Float3 UpVector;

//     static constexpr auto TYPE = COMPONENT_TYPE::Plane;
// };


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
        {}

        ApplyForceInfo(const ApplyForceInfo& other) :
            MultiplyByMass(other.MultiplyByMass), Callback(other.Callback)
        {
            if(other.OptionalName)
                OptionalName = std::make_unique<std::string>(*other.OptionalName);
        }

        ApplyForceInfo(ApplyForceInfo&& other) :
            OptionalName(move(other.OptionalName)),
            MultiplyByMass(std::move(other.MultiplyByMass)), Callback(move(other.Callback))
        {}

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
        //! \todo Remove if this is still unneeded
        GameWorld* world;
        //! \todo Replace this with automatically retrieved Position component from this entity
        Position& updatepos;
    };

public:
    inline Physics(const Data& args) :
        Component(TYPE), ThisEntity(args.id), _Position(args.updatepos)
    {}

    DLLEXPORT ~Physics();

    //! \brief Destroys the physical body
    DLLEXPORT void Release(PhysicalWorld* world);

    //! \brief Use this to create a body for this component once Collision is set
    //! \param physicsmaterialid Retrieve from the same world with
    //! `GameWorld::GetPhysicalMaterial`. -1 to use default material
    //! \todo This should be changed so that the Physics constructor would create the physics
    //! body
    //! \warning Only the physical world from the GameWorld that this entity is in may be used
    //! here
    DLLEXPORT PhysicsBody::pointer CreatePhysicsBody(PhysicalWorld* world,
        const PhysicsShape::pointer& shape, float mass, int physicsmaterialid = -1);

    //! \brief Updates the shape. This can be the same pointer as before. BUT if the shape is
    //! modified this MUST be called before physics is simulated the next time
    DLLEXPORT bool ChangeShape(PhysicalWorld* world, const PhysicsShape::pointer& shape);

    //! \brief Syncs this physics body to a changed position.
    //!
    //! Call after making changes to Position component if you don't want this physics
    //! body to overwrite the change on next tick.
    DLLEXPORT void JumpTo(Position& target);

    //! \brief Applies physical state from holder object
    DLLEXPORT void ApplyPhysicalState(const BasePhysicsData& data);

    //! \brief Adds all applied forces together
    Float3 _GatherApplyForces(const float& mass);

    DLLEXPORT float GetMass() const;

    inline PhysicsBody* GetBody() const
    {
        return Body.get();
    }

    //
    // Script wrappers
    //

    inline PhysicsBody* GetBodyWrapper() const
    {
        if(Body)
            Body->AddRef();
        return Body.get();
    }

    inline bool ChangeShapeWrapper(PhysicalWorld* world, PhysicsShape* shape)
    {
        return ChangeShape(world, ReferenceCounted::WrapPtr(shape));
    }

    inline PhysicsBody* CreatePhysicsBodyWrapper(
        PhysicalWorld* world, PhysicsShape* shape, float mass, int physicsmaterialid)
    {
        auto body = CreatePhysicsBody(
            world, ReferenceCounted::WrapPtr(shape), mass, physicsmaterialid);

        if(body)
            body->AddRef();
        return body.get();
    }

    REFERENCE_HANDLE_UNCOUNTED_TYPE(Physics);

private:
    PhysicsBody::pointer Body;

public:
    //! For passing to PhysicsBody::SetOwningEntity
    ObjectID ThisEntity;

    // //! Used to access gravity data
    // GameWorld* World = nullptr;

    //! Physics object requires a position
    Position& _Position;

    static constexpr auto TYPE = COMPONENT_TYPE::Physics;
};

// class ManualObject : public Component {
// public:
//     DLLEXPORT ManualObject(bs::Scene* scene);

//     DLLEXPORT void Release(bs::Scene* scene);

//     REFERENCE_HANDLE_UNCOUNTED_TYPE(ManualObject);

//     Ogre::ManualObject* Object = nullptr;

//     //! When not empty the ManualObject has been created into an actual mesh
//     //! that needs to be destroyed on release
//     //! \note The Object can be directly added to a scene so this may be empty even
//     //! if the Object is created
//     std::string CreatedMesh;

//     static constexpr auto TYPE = COMPONENT_TYPE::ManualObject;
// };


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
//     DLLEXPORT void Release(bs::Scene* scene);

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
    //! \version This now takes the horizontal instead of vertical FOV
    inline Camera(uint8_t fov = 90, bool soundperceiver = true) :
        Component(TYPE), FOV(fov), SoundPerceiver(soundperceiver)
    {}

    REFERENCE_HANDLE_UNCOUNTED_TYPE(Camera);

    //! Horizontal (ie. "normal") field of view
    uint16_t FOV;

    bool SoundPerceiver;
    // TODO: orthographic
    // bool Orthographic;

    static constexpr auto TYPE = COMPONENT_TYPE::Camera;
};

} // namespace Leviathan

#ifdef LEAK_INTO_GLOBAL
using ApplyForceInfo = Leviathan::Physics::ApplyForceInfo;
#endif
