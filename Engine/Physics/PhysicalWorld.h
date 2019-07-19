// Leviathan Game Engine
// Copyright (c) 2012-2018 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "Common/ThreadSafe.h"
#include "Common/Types.h"
#include "PhysicsBody.h"
#include "PhysicsShape.h"

#include <functional>

// Bullet forward declarations
class btCollisionShape;
class btRigidBody;
class btDefaultCollisionConfiguration;
class btCollisionDispatcher;
class btBroadphaseInterface;
class btSequentialImpulseConstraintSolver;
class btDiscreteDynamicsWorld;
class btDynamicsWorld;
class btPersistentManifold;
class btManifoldPoint;
class btCollisionObject;

#ifdef BT_USE_DOUBLE_PRECISION
using btScalar = double;
#else
using btScalar = float;
#endif


namespace Leviathan {

class LeviathanPhysicsOverlapFilter;
struct PhysMaterialDataPair;

constexpr auto PHYSICS_BASE_GRAVITY = -9.81f;

// //! \brief Base class for custom joint types defined for use by this class
// //! \note If these should be able to be accessed from elsewhere, move this to a new file
// class BaseCustomJoint {
// public:
//     //! This helps with allowing all custom types to be destroyed with a single callback
//     DLLEXPORT virtual ~BaseCustomJoint() {}

//     DLLEXPORT static void JointDestructorCallback(const NewtonJoint* joint);
// };

// TODO: need to find some way to fake this with bullet
// int SingleBodyUpdate(
//     const NewtonWorld* const newtonWorld, const void* islandHandle, int bodyCount);

class PhysicalWorld {
    // friend int SingleBodyUpdate(
    //     const NewtonWorld* const newtonWorld, const void* islandHandle, int bodyCount);
public:
    DLLEXPORT PhysicalWorld(GameWorld* owner, PhysicsMaterialManager* physicscallbacks);
    DLLEXPORT ~PhysicalWorld();

    //! \brief Advances the simulation the specified amount of time
    DLLEXPORT void SimulateWorld(float secondspassed, int maxsubsteps = 4);

    // ------------------------------------ //
    // Physics collision creation
    // NOTE: all created collisions HAVE to be destroyed after all bodies using them are
    // destroyed
    DLLEXPORT PhysicsShape::pointer CreateCompound();

    DLLEXPORT PhysicsShape::pointer CreateSphere(float radius);

    //! The dimensions are halfwidths of the box along each axis
    DLLEXPORT PhysicsShape::pointer CreateBox(
        float xdimension, float ydimension, float zdimension);


    // ------------------------------------ //
    // Physics constraint creation
    //! \brief Constraints body to a 2d plane of movement specified by its normal
    // DLLEXPORT NewtonJoint* Create2DJoint(NewtonBody* body, const Float3& planenormal);

    // ------------------------------------ //

    //! \note DestroyBody must be called to remove the created body from this world
    //! \param mass If 0 then this is a static body
    DLLEXPORT PhysicsBody::pointer CreateBodyFromCollision(const PhysicsShape::pointer& shape,
        float mass, PhysicsPositionProvider* positionsynchronization,
        int physicsmaterialid = -1);

    //! \brief Applies a changed shape to a body
    //! \todo Find out if it is required to remove and add the body back to the world (and if
    //! that can be done in a physics callback)
    DLLEXPORT bool ChangeBodyShape(
        const PhysicsBody::pointer& body, const PhysicsShape::pointer& shape);

    //! \brief Destroys a physics body
    //!
    //! May not be called while a physics update is in progress
    DLLEXPORT bool DestroyBody(PhysicsBody* body);


    //! \brief Finds the information for contact between objects with two materials
    const PhysMaterialDataPair* GetMaterialPair(int id1, int id2) const;

    DLLEXPORT inline GameWorld* GetGameWorld()
    {
        return OwningWorld;
    }

    //
    // Script wrappers
    //
    inline PhysicsShape* CreateSphereWrapper(float radius)
    {
        auto obj = CreateSphere(radius);

        if(obj)
            obj->AddRef();

        return obj.get();
    }

    inline PhysicsShape* CreateCompoundWrapper()
    {
        auto obj = CreateCompound();

        if(obj)
            obj->AddRef();

        return obj.get();
    }

protected:
    static void OnPhysicsSubStep(btDynamicsWorld* world, btScalar timeStep);

    //! \brief Calls appropriate material callbacks. This is called once per contact manifold
    //! that has penetrating points
    void OnManifoldWithContact(btPersistentManifold* contactManifold,
        const btManifoldPoint& contactPoint, const btCollisionObject* objA,
        const btCollisionObject* objB);

protected:
    //! Total amount of seconds required to be simulated
    float PassedTimeTotal = 0;

    GameWorld* OwningWorld;
    PhysicsMaterialManager* PhysicsMaterials;

    //! This is a small sanity check for preventing destroying physics bodies during a tick
    bool PhysicsUpdateInProgress = false;

    // Bullet resources
    std::unique_ptr<btDefaultCollisionConfiguration> CollisionConfiguration;

    std::unique_ptr<btCollisionDispatcher> Dispatcher;

    std::unique_ptr<btBroadphaseInterface> OverlappingPairCache;

    std::unique_ptr<btSequentialImpulseConstraintSolver> Solver;

    std::unique_ptr<btDiscreteDynamicsWorld> DynamicsWorld;

    std::unique_ptr<LeviathanPhysicsOverlapFilter> OverlapFilter;

    //! We need to keep the physic bodies alive (guaranteed) until they are destroyed
    std::vector<PhysicsBody::pointer> PhysicsBodies;

    // //! Used for resimulation
    // //! \todo Potentially allow this to be a vector
    // NewtonBody* ResimulatedBody = nullptr;
};

} // namespace Leviathan
