// ------------------------------------ //
#include "PhysicalWorld.h"

#include "Events/EventHandler.h"
#include "PhysicsDebugDrawer.h"
#include "PhysicsMaterialManager.h"

#include "Engine.h"
#include "TimeIncludes.h"

#include "BulletDynamics/ConstraintSolver/btFixedConstraint.h"
#include "btBulletDynamicsCommon.h"
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

        // TODO: remove this check once it is confirmed that both objects must always exist
        if(obj0 && obj1) {

            PhysicsBody* body1 = static_cast<PhysicsBody*>(obj0->getUserPointer());
            PhysicsBody* body2 = static_cast<PhysicsBody*>(obj1->getUserPointer());

            if(body1 && body2) {

                const auto materialID1 = body1->GetPhysicalMaterialID();
                const auto materialID2 = body2->GetPhysicalMaterialID();

                // Find contact callbacks
                if(materialID1 >= 0 && materialID2 >= 0) {

                    auto pair = World->GetMaterialPair(materialID1, materialID2);

                    if(pair && pair->AABBCallback) {

                        return pair->AABBCallback(*World, *body1, *body2);
                    }
                }
            } else {
                LOG_ERROR("Physics body doesn't have user pointer");
            }

        } else {
            LOG_ERROR("Physics btCollisionObject doesn't exist");
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
    // It is an error to not release a body
    if(!PhysicsBodies.empty()) {
        LOG_ERROR("PhysicalWorld: not all bodies have been destroyed, alive count: " +
                  std::to_string(PhysicsBodies.size()));
    }

    // All bodies need to be destroyed here
    while(!PhysicsBodies.empty()) {
        DestroyBody(PhysicsBodies.back().get());
    }

    // We can rely on the default delete order and everything is fine
}
// ------------------------------------ //
DLLEXPORT void PhysicalWorld::SimulateWorld(float secondspassed, int maxsubsteps /*= 4*/)
{
    PhysicsUpdateInProgress = true;

    DynamicsWorld->stepSimulation(secondspassed, maxsubsteps);

    PhysicsUpdateInProgress = false;

    if(DebugDrawer) {
        DebugDrawer->OnBeginDraw();
        DynamicsWorld->debugDrawWorld();
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

        const int numContacts = contactManifold->getNumContacts();

        if(numContacts < 1)
            continue;

        const btCollisionObject* objA = contactManifold->getBody0();
        const btCollisionObject* objB = contactManifold->getBody1();

        // TODO: remove this check once it is confirmed that both objects must always exist
        if(objA && objB) {

            PhysicsBody* body1 = static_cast<PhysicsBody*>(objA->getUserPointer());
            PhysicsBody* body2 = static_cast<PhysicsBody*>(objB->getUserPointer());

            // TODO: remove this check after confirming neither is ever null
            if(body1 && body2) {

                // Find matching materials
                const auto materialID1 = body1->GetPhysicalMaterialID();
                const auto materialID2 = body2->GetPhysicalMaterialID();

                // Find contact callbacks
                if(materialID1 >= 0 && materialID2 >= 0) {

                    auto pair = leviathanWorld->GetMaterialPair(materialID1, materialID2);

                    if(!pair)
                        continue;

                    if(pair->ManifoldCallback) {

                        pair->ManifoldCallback(
                            *leviathanWorld, *body1, *body2, *contactManifold);

                        // Maybe release or clear here allows rechecking the AABB overlap
                        // dispatcher->releaseManifold()
                    }

                    bool handled = false;

                    for(int j = 0; j < numContacts; j++) {

                        const btManifoldPoint& contactPoint =
                            contactManifold->getContactPoint(j);

                        if(contactPoint.getDistance() < 0.f) {

                            if(handled)
                                continue;

                            handled = true;
                            leviathanWorld->OnManifoldWithContact(
                                contactManifold, contactPoint, *pair, body1, body2);
                        }
                    }
                }
            }
        }
    }
}

void PhysicalWorld::OnManifoldWithContact(btPersistentManifold* contactManifold,
    const btManifoldPoint& contactPoint, const PhysMaterialDataPair& pair, PhysicsBody* body1,
    PhysicsBody* body2)
{
    // Actual points touching
    // const btVector3& contactPointA = contactPoint.getPositionWorldOnA();
    // const btVector3& contactPointB = contactPoint.getPositionWorldOnB();
    // const btVector3& normalOnB = contactPoint.m_normalWorldOnB;

    if(pair.ContactCallback) {

        pair.ContactCallback(*this, *body1, *body2);
    }
}

const PhysMaterialDataPair* PhysicalWorld::GetMaterialPair(int id1, int id2) const
{
    const auto material1 = PhysicsMaterials->GetMaterial(id1);

    const PhysMaterialDataPair* pair = nullptr;

    // The logs in this method will absolutely trash performance if they get hit as there will
    // be tons of calls with the same parameters

    if(material1) {
        pair = material1->GetPairWith(id2);
    } else {
        LOG_ERROR("PhysicsBody has invalid material ID: " + std::to_string(id1));
    }

    if(!pair) {

        const auto material2 = PhysicsMaterials->GetMaterial(id2);

        if(material2) {
            pair = material2->GetPairWith(id1);
        } else {
            LOG_ERROR("PhysicsBody has invalid material ID: " + std::to_string(id2));
        }
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
    if(std::isnan(radius)) {

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

DLLEXPORT PhysicsShape::pointer PhysicalWorld::CreateCone(float radius, float height)
{
    return PhysicsShape::MakeShared<PhysicsShape>(
        std::make_unique<btConeShape>(radius, height));
}
// ------------------------------------ //
DLLEXPORT PhysicsConstraint::pointer PhysicalWorld::CreateFixedConstraint(
    const PhysicsBody::pointer& a, const PhysicsBody::pointer& b, const Float3& aoffset,
    const Float4& aorientation, const Float3& boffset, const Float4& borientation)
{
    if(!a || !a->GetBody() || !b || !b->GetBody())
        return nullptr;

    btTransform aTransform;
    aTransform.setIdentity();

    aTransform.setRotation(aorientation);
    aTransform.setOrigin(aoffset);

    btTransform bTransform;
    bTransform.setIdentity();

    bTransform.setRotation(borientation);
    bTransform.setOrigin(boffset);

    auto newConstraint = PhysicsConstraint::MakeShared<PhysicsConstraint>(
        new btFixedConstraint(*a->GetBody(), *b->GetBody(), aTransform, bTransform), a.get(),
        b.get());

    DynamicsWorld->addConstraint(newConstraint->GetConstraint());
    PhysicsConstraints.push_back(newConstraint);

    return newConstraint;
}
// ------------------------------------ //
DLLEXPORT bool PhysicalWorld::DestroyConstraint(PhysicsConstraint* constraint)
{
    if(PhysicsUpdateInProgress) {
        LOG_FATAL(
            "PhysicalWorld: DestroyConstraint: called while physics update is in progress");
        return false;
    }

    // Remove from existing constraints and the world
    for(auto iter = PhysicsConstraints.begin(); iter != PhysicsConstraints.end(); ++iter) {
        if(iter->get() == constraint) {

            DynamicsWorld->removeConstraint(constraint->GetConstraint());

            constraint->DetachResources();
            PhysicsConstraints.erase(iter);
            return true;
        }
    }

    // This is not serious as destroying bodies can trigger destroying constraints
    return false;
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
        const Quaternion* orientation;
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
    std::unique_ptr<PhysicsDataBridge> positionBridge;

    if(positionsynchronization)
        positionBridge = std::make_unique<PhysicsDataBridge>(positionsynchronization);

    btRigidBody::btRigidBodyConstructionInfo info(
        mass, positionBridge.get(), shape->GetShape(), localInertia);

    auto body = PhysicsBody::MakeShared<PhysicsBody>(std::make_unique<btRigidBody>(info), mass,
        shape, std::move(positionBridge), physicsmaterialid);

    if(!body->GetBody()) {
        LOG_ERROR("PhysicalWorld: CreateBodyFromCollision: failed to create bullet body");
        return nullptr;
    }

    // Apply material
    if(physicsmaterialid != -1 && PhysicsMaterials) {
        auto material = PhysicsMaterials->GetMaterial(physicsmaterialid);

        if(!material) {
            LOG_ERROR(
                "PhysicalWorld: CreateBodyFromCollision: can't find physics material id: " +
                std::to_string(physicsmaterialid));
            body->SetPhysicalMaterialID(-1);
        } else {

            body->ApplyMaterial(*material);
        }
    }

    // Dirty hack to make bodies not sleep. This makes apply force work (another approach would
    // be to wake any body that has force applied to it)
    body->GetBody()->setActivationState(DISABLE_DEACTIVATION);

    if(PhysicsUpdateInProgress) {
        // This might be okay...
        // DEBUG_BREAK;
    }

    DynamicsWorld->addRigidBody(body->GetBody());

    // Make sure it is alive as long as it is in the world
    PhysicsBodies.push_back(body);
    return body;
}

DLLEXPORT bool PhysicalWorld::ChangeBodyShape(
    const PhysicsBody::pointer& body, const PhysicsShape::pointer& shape)
{
    if(!body || !shape || !body->GetBody())
        return false;

    // Debug safety code
    bool found = false;
    for(auto iter = PhysicsBodies.begin(); iter != PhysicsBodies.end(); ++iter) {
        if(iter->get() == body.get()) {

            found = true;
            break;
        }
    }

    if(!found) {
        LOG_ERROR("PhysicalWorld: ChangeBodyShape: passed body not part of this world");
        DEBUG_BREAK;
        return false;
    }

    // This might be fine as it will be added back in
    if(body->GetBody()->getNumConstraintRefs() > 0) {
        // "undestroyed constraints on entity";
    }

    if(PhysicsUpdateInProgress) {
        // This might be okay...
        // DEBUG_BREAK;
    }

    DynamicsWorld->removeRigidBody(body->GetBody());

    body->ApplyShapeChange(shape);

    DynamicsWorld->addRigidBody(body->GetBody());

    return true;
}

DLLEXPORT bool PhysicalWorld::DestroyBody(PhysicsBody* body)
{
    if(PhysicsUpdateInProgress) {
        LOG_FATAL("PhysicalWorld: DestroyBody: called while physics update is in progress");
        return false;
    }

    // Remove from alive bodies. This also checks that the body is in this world
    for(auto iter = PhysicsBodies.begin(); iter != PhysicsBodies.end(); ++iter) {

        if(iter->get() == body) {

            // Destroy all constraints we can see
            while(!body->GetConstraints().empty()) {
                DestroyConstraint(body->GetConstraints().front().get());
            }

            if(body->GetBody()->getNumConstraintRefs() > 0) {

                LOG_ERROR("PhysicalWorld: DestroyBody: body has undestroyed constraints, "
                          "can't safely destroy");
                return false;
            }

            DynamicsWorld->removeRigidBody(body->GetBody());

            body->DetachResources();
            PhysicsBodies.erase(iter);
            return true;
        }
    }

    LOG_ERROR("PhysicalWorld: DestroyBody: called with body that wasn't in this world");
    return false;
}
// ------------------------------------ //
DLLEXPORT void PhysicalWorld::SetDebugDrawer(const std::shared_ptr<PhysicsDebugDrawer>& drawer)
{
    DebugDrawer = drawer;

    if(DebugDrawer) {
        DynamicsWorld->setDebugDrawer(DebugDrawer.get());
    } else {
        DynamicsWorld->setDebugDrawer(nullptr);
    }
}
