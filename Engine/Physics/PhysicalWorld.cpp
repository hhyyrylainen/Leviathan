// ------------------------------------ //
#include "PhysicalWorld.h"

#include "../TimeIncludes.h"
#include "Engine.h"
#include "Events/EventHandler.h"
// #include "NewtonConversions.h"
#include "PhysicsMaterialManager.h"

#include <bullet/btBulletDynamicsCommon.h>
using namespace Leviathan;
// ------------------------------------ //
namespace Leviathan {
//! \brief Handles AABB material callbacks
class LeviathanPhysicsOverlapFilter : public btOverlapFilterCallback {
public:
    LeviathanPhysicsOverlapFilter(PhysicalWorld* world) : World(world) {}

    bool needBroadphaseCollision(
        btBroadphaseProxy* proxy0, btBroadphaseProxy* proxy1) const override
    {
        bool collides = (proxy0->m_collisionFilterGroup & proxy1->m_collisionFilterMask) != 0;
        collides =
            collides && (proxy1->m_collisionFilterGroup & proxy0->m_collisionFilterMask);

        if(!collides)
            return false;

        // Custom material check (this is after the fast inbuilt bullet mask check mimicking
        // check)
        const btCollisionObject* obj0 =
            static_cast<btCollisionObject*>(proxy0->m_clientObject);
        const btCollisionObject* obj1 =
            static_cast<btCollisionObject*>(proxy1->m_clientObject);

        PhysicsBody* body1 = static_cast<PhysicsBody*>(obj0->getUserPointer());
        PhysicsBody* body2 = static_cast<PhysicsBody*>(obj1->getUserPointer());

        const auto materialID1 = body1->GetPhysicalMaterialID();
        const auto materialID2 = body2->GetPhysicalMaterialID();

        // Find contact callbacks
        if(materialID1 >= 0 && materialID2 >= 0) {

            auto pair = World->GetMaterialPair(materialID1, materialID2);

            if(pair && pair->AABBCallback) {

                return pair->AABBCallback(*body1, *body2);
            }
        }

        // Needs collision
        return true;
    }

private:
    PhysicalWorld* World;
};
} // namespace Leviathan


DLLEXPORT PhysicalWorld::PhysicalWorld(
    GameWorld* owner, PhysicsMaterialManager* physicscallbacks) :
    OwningWorld(owner),
    PhysicsMaterials(physicscallbacks),
    OverlapFilter(std::make_unique<LeviathanPhysicsOverlapFilter>(this))
{
    // Setup physics world //
    CollisionConfiguration = std::make_unique<btDefaultCollisionConfiguration>();

    // Non-parallel dispatcher
    Dispatcher = std::make_unique<btCollisionDispatcher>(CollisionConfiguration.get());

    // According to docs this is a good general broadphase
    OverlappingPairCache = std::make_unique<btDbvtBroadphase>();

    // Non-parallel solver
    Solver = std::make_unique<btSequentialImpulseConstraintSolver>();

    DynamicsWorld = std::make_unique<btDiscreteDynamicsWorld>(Dispatcher.get(),
        OverlappingPairCache.get(), Solver.get(), CollisionConfiguration.get());

    // Register required callbacks
    DynamicsWorld->setInternalTickCallback(&PhysicalWorld::OnPhysicsSubStep);
    // Not sure which one of these is better to use for AABB overlap
    DynamicsWorld->getPairCache()->setOverlapFilterCallback(OverlapFilter.get());
    // Dispatcher->setNearCallback(&PhysicalWorld::PhysicsNearCallback);

    // Set gravity. Having this set on worlds that want multiple different gravities might be
    // an issue
    DynamicsWorld->setGravity(btVector3(0, PHYSICS_BASE_GRAVITY, 0));

    // Set us as the user data (this is done last so that nothing overwrites this) //
    DynamicsWorld->setWorldUserInfo(this);

    LEVIATHAN_ASSERT(DynamicsWorld->getWorldUserInfo() != nullptr, "userinfo set failed");
}

DLLEXPORT PhysicalWorld::~PhysicalWorld()
{
    // All bodies need to be destroyed here

    // We can rely on the default delete order and everything is fine
}
// ------------------------------------ //
DLLEXPORT void PhysicalWorld::SimulateWorld(float secondspassed, int splits /*= 1*/)
{
    if(splits <= 0) {

        // TODO: report error?
        splits = 1;
    }

    const float timeStep = secondspassed / splits;

    for(int i = 0; i < splits; ++i) {

        Engine::Get()->GetEventHandler()->CallEvent(new Event(
            EVENT_TYPE_PHYSICS_BEGIN, new PhysicsStartEventData(timeStep, OwningWorld)));

        // We do our own splitting to timesteps
        DynamicsWorld->stepSimulation(timeStep, 0);
    }
}

void PhysicalWorld::OnPhysicsSubStep(btDynamicsWorld* world, btScalar timeStep)
{
    auto leviathanWorld = static_cast<PhysicalWorld*>(world->getWorldUserInfo());

    // Skip if no physics materials
    if(!leviathanWorld->PhysicsMaterials)
        return;

    btDispatcher* dispatcher = world->getDispatcher();

    // Check all active contacts as to whether something should be done
    const int numManifolds = dispatcher->getNumManifolds();
    for(int i = 0; i < numManifolds; i++) {

        btPersistentManifold* contactManifold = dispatcher->getManifoldByIndexInternal(i);

        const btCollisionObject* objA = contactManifold->getBody0();
        const btCollisionObject* objB = contactManifold->getBody1();

        const int numContacts = contactManifold->getNumContacts();

        bool handled = false;

        for(int j = 0; j < numContacts; j++) {

            const btManifoldPoint& contactPoint = contactManifold->getContactPoint(j);

            if(contactPoint.getDistance() < 0.f) {

                if(handled)
                    continue;

                handled = true;
                leviathanWorld->OnManifoldWithContact(
                    contactManifold, contactPoint, objA, objB);
            }
        }
    }
}

void PhysicalWorld::OnManifoldWithContact(btPersistentManifold* contactManifold,
    const btManifoldPoint& contactPoint, const btCollisionObject* objA,
    const btCollisionObject* objB)
{
    // Actual points touching
    // const btVector3& contactPointA = contactPoint.getPositionWorldOnA();
    // const btVector3& contactPointB = contactPoint.getPositionWorldOnB();
    // const btVector3& normalOnB = contactPoint.m_normalWorldOnB;

    // Find matching materials
    PhysicsBody* body1 = static_cast<PhysicsBody*>(objA->getUserPointer());
    PhysicsBody* body2 = static_cast<PhysicsBody*>(objB->getUserPointer());

    const auto materialID1 = body1->GetPhysicalMaterialID();
    const auto materialID2 = body2->GetPhysicalMaterialID();

    // Find contact callbacks
    if(materialID1 >= 0 && materialID2 >= 0) {

        auto pair = GetMaterialPair(materialID1, materialID2);

        if(pair && pair->ContactCallback) {

            pair->ContactCallback(*body1, *body2);
        }
    }
}

const PhysMaterialDataPair* PhysicalWorld::GetMaterialPair(int id1, int id2) const
{
    const auto material1 = PhysicsMaterials->GetMaterial(id1);
    const PhysMaterialDataPair* pair = material1->GetPairWith(id2);

    if(!pair) {

        const auto material2 = PhysicsMaterials->GetMaterial(id2);
        pair = material2->GetPairWith(id1);
    }

    return pair;
}
// ------------------------------------ //
// int Leviathan::SingleBodyUpdate(
//     const NewtonWorld* const newtonWorld, const void* islandHandle, int bodyCount)
// {
//     PhysicalWorld* pworld =
//         reinterpret_cast<PhysicalWorld*>(NewtonWorldGetUserData(newtonWorld));

//     for(int i = 0; i < bodyCount; i++) {

//         if(NewtonIslandGetBody(islandHandle, i) == pworld->ResimulatedBody) {

//             // Target body is part of this collision, simulate it //
//             return 1;
//         }
//     }

//     // Wasn't the target body, ignore //
//     return 0;
// }
// ------------------------------------ //
DLLEXPORT PhysicsShape::pointer PhysicalWorld::CreateCompound()
{
    return PhysicsShape::MakeShared<PhysicsShape>(std::make_unique<btCompoundShape>());
}

DLLEXPORT PhysicsShape::pointer PhysicalWorld::CreateSphere(float radius)
{
#ifdef CHECK_FOR_NANS
    bool matrixNans = false;
    for(size_t i = 0; i < 16; ++i) {
        if(std::isnan(*(offset[0] + i))) {
            matrixNans = true;
            break;
        }
    }

    if(std::isnan(radius) || matrixNans) {

        DEBUG_BREAK;
        throw std::runtime_error("CreateSphere has NaNs in it!");
    }
#endif // CHECK_FOR_NANS

    return PhysicsShape::MakeShared<PhysicsShape>(std::make_unique<btSphereShape>(radius));
}

DLLEXPORT PhysicsShape::pointer PhysicalWorld::CreateBox(
    float xdimension, float ydimension, float zdimension)
{
    return PhysicsShape::MakeShared<PhysicsShape>(
        std::make_unique<btBoxShape>(btVector3(xdimension, ydimension, zdimension)));
}
// ------------------------------------ //
DLLEXPORT PhysicsBody::pointer PhysicalWorld::CreateBodyFromCollision(
    const PhysicsShape::pointer& shape, float mass,
    PhysicsPositionProvider* positionsynchronization, int physicsmaterialid /*= -1*/)
{
    if(!shape)
        return nullptr;

    btTransform transform;
    transform.setIdentity();

    if(positionsynchronization) {
        const Float3* position;
        const Float4* orientation;
        positionsynchronization->GetPositionDataForPhysics(position, orientation);

        transform.setRotation(*orientation);
        transform.setOrigin(*position);
    }

    // rigidbody is dynamic if and only if mass is non zero, otherwise static
    const bool isDynamic = mass > 0;

    btVector3 localInertia(0, 0, 0);
    if(isDynamic)
        shape->GetShape()->calculateLocalInertia(mass, localInertia);

    // To not have to lookup all positions all the time we use the position synchronization
    // object
    btRigidBody::btRigidBodyConstructionInfo info(
        mass, positionsynchronization, shape->GetShape(), localInertia);

    auto body = PhysicsBody::MakeShared<PhysicsBody>(std::make_unique<btRigidBody>(info), mass,
        shape, positionsynchronization, physicsmaterialid);

    // Apply material
    if(physicsmaterialid != -1 && PhysicsMaterials) {

        auto material = PhysicsMaterials->GetMaterial(physicsmaterialid);

        if(!material) {
            LOG_ERROR(
                "PhysicalWorld: CreateBodyFromCollision: can't find physics material id: " +
                std::to_string(physicsmaterialid));
        } else {

            body->ApplyMaterial(*material);
        }
    }

    DynamicsWorld->addRigidBody(body->GetBody());
    return body;
}

DLLEXPORT bool PhysicalWorld::ChangeBodyShape(
    const PhysicsBody::pointer& body, const PhysicsShape::pointer& shape)
{
    if(!body || !shape || !body->GetBody())
        return false;

    // const auto oldTransform = body->GetBody()->getCenterOfMassTransform();

    DynamicsWorld->removeRigidBody(body->GetBody());

    body->ApplyShapeChange(shape);

    DynamicsWorld->addRigidBody(body->GetBody());
    return true;
}

DLLEXPORT void PhysicalWorld::DestroyBody(PhysicsBody* body)
{
    DynamicsWorld->removeRigidBody(body->GetBody());
    body->DetachResources();
}

// ------------------------------------ //
// DLLEXPORT void BaseCustomJoint::JointDestructorCallback(const NewtonJoint* joint)
// {
//     void* user = NewtonJointGetUserData(joint);

//     if(user)
//         delete static_cast<BaseCustomJoint*>(user);
// }

// class Joint2D : public BaseCustomJoint {
// public:
//     Joint2D(
//         const Ogre::Vector3& planenormal, const Ogre::Vector3& planeorigin, NewtonBody*
//         body) : Normal(planenormal), Origin(planeorigin), Body(body)
//     {}

//     //! \note This is continually called by newton to get what the object needs to do
//     static void SubmitConstraints(
//         const NewtonJoint* const joint, dFloat timestep, int threadIndex)
//     {
//         void* rawData = NewtonJointGetUserData(joint);
//         Joint2D* self = static_cast<Joint2D*>(static_cast<BaseCustomJoint*>(rawData));

//         dFloat matrix[16];
//         NewtonBodyGetMatrix(self->Body, &matrix[0]);
//         auto matrixPos = ExtractNewtonMatrixTranslation(matrix);

//         // this line clamps the origin to the plane
//         NewtonUserJointAddLinearRow(joint, &matrixPos.X, &self->Origin.x, &self->Normal.x);

//         // we can prevent rotation that takes any points out of the plane by clamping a
//         point
//         // on the object that is on the line through the origin of the object with the same
//         // vector as the plane normal.  The clamp is to another plane that is parallel to
//         the
//         // first, offset by the plane normal vector.  Rotations around either of the axes
//         // orthogonal to the plane normal will be prevented because they take the object
//         point
//         // off that parallel plane.

//         // TODO: find a faster way than transposing here
//         auto bodyMatrix = NewtonMatrixToOgre(matrix);

//         Ogre::Vector3 object_point = bodyMatrix * self->Normal;
//         auto world_point = self->Origin + self->Normal;
//         NewtonUserJointAddLinearRow(joint, &object_point.x, &world_point.x,
//         &self->Normal.x);
//     }

//     const Ogre::Vector3 Normal;
//     const Ogre::Vector3 Origin;
//     const NewtonBody* Body;
// };

// // ------------------------------------ //
// DLLEXPORT NewtonJoint* PhysicalWorld::Create2DJoint(
//     NewtonBody* body, const Float3& planenormal)
// {
//     // Method from
//     http://newtondynamics.com/wiki/index.php5?title=CJ_2D_Joint_planar_rotation dFloat
//     matrix[16]; NewtonBodyGetMatrix(body, &matrix[0]); const auto planeOrigin =
//     ExtractNewtonMatrixTranslation(matrix);

//     NewtonJoint* joint =
//         NewtonConstraintCreateUserJoint(World, 6, Joint2D::SubmitConstraints, body,
//         nullptr);

//     Joint2D* jointData = new Joint2D(planenormal, planeOrigin, body);

//     NewtonJointSetUserData(joint, static_cast<BaseCustomJoint*>(jointData));

//     NewtonJointSetDestructor(joint, BaseCustomJoint::JointDestructorCallback);
//     return joint;
// }

// ------------------------------------ //
