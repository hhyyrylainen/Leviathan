// ------------------------------------ //
#include "PhysicsBody.h"

#include "Common/Types.h"
#include "Exceptions.h"
#include "Utility/Convert.h"

#include "BulletCollision/CollisionShapes/btCollisionShape.h"
#include "BulletDynamics/Dynamics/btRigidBody.h"

using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT PhysicsBody::PhysicsBody(std::unique_ptr<btRigidBody>&& body, float mass,
    const PhysicsShape::pointer& shape, PhysicsPositionProvider* positionsynchronization,
    int materialid) :
    Body(std::move(body)),
    Mass(mass), Shape(shape), PositionUpdate(positionsynchronization),
    PhysicalMaterialID(materialid)
{
    if(!Body)
        throw InvalidArgument("null rigid body passed to PhysicsBody");

    // Add this as user data //
    Body->setUserPointer(this);
}

DLLEXPORT void PhysicsBody::DetachResources()
{
    Body.reset();
    // TODO: unhook position synchronization
}
// ------------------------------------ //
DLLEXPORT void PhysicsBody::ApplyShapeChange(const PhysicsShape::pointer& shape)
{
    if(!shape)
        return;

    // This is only called after the physical world has verified that this has a body
    Body->setCollisionShape(shape->GetShape());
    Shape = shape;

    // Recalculate inertia
    const bool isDynamic = Mass > 0;

    btVector3 localInertia(0, 0, 0);
    if(isDynamic)
        Shape->GetShape()->calculateLocalInertia(Mass, localInertia);

    Body->setMassProps(Mass, localInertia);
}
// ------------------------------------ //
DLLEXPORT bool PhysicsBody::SetPosition(const Float3& pos, const Float4& orientation)
{
    if(!Body)
        throw InvalidArgument("PhysicsBody has no longer an internal physics engine body");

    // Safety check. Can be disabled in release builds if this is a performance issue
    if(pos.HasInvalidValues() || orientation.HasInvalidValues()) {

        std::stringstream msg;
        msg << "SetPosition call had at least one value with non-finite value, pos: "
            << Convert::ToString(pos) << " orientation: " << Convert::ToString(orientation);
        LOG_ERROR("PhysicsBody: " + msg.str());
        throw InvalidArgument(msg.str());
    }

    btTransform transform;
    // TODO: check is this required
    transform.setIdentity();

    transform.setRotation(orientation);
    transform.setOrigin(pos);

    Body->setCenterOfMassTransform(transform);
    return true;
}

DLLEXPORT bool PhysicsBody::SetOnlyOrientation(const Float4& orientation)
{
    if(!Body)
        throw InvalidArgument("PhysicsBody has no longer an internal physics engine body");

    // Safety check. Can be disabled in release builds if this is a performance issue
    if(orientation.HasInvalidValues()) {

        std::stringstream msg;
        msg << "SetOnlyOrientation call had at least one value with non-finite value, "
               "orientation: "
            << Convert::ToString(orientation);
        LOG_ERROR("PhysicsBody: " + msg.str());
        throw InvalidArgument(msg.str());
    }

    btTransform pos = Body->getCenterOfMassTransform();

    pos.setRotation(orientation);

    Body->setCenterOfMassTransform(pos);
    return true;
}
// ------------------------------------ //
DLLEXPORT void PhysicsBody::GiveImpulse(const Float3& deltaspeed, const Float3& point
    /*= Float3(0)*/)
{
    if(!Body)
        throw InvalidArgument("PhysicsBody has no longer an internal physics engine body");


    // Safety check. Can be disabled in release builds if this is a performance issue
    if(deltaspeed.HasInvalidValues() || point.HasInvalidValues()) {

        std::stringstream msg;
        msg << "GiveImpulse call had at least one value with non-finite value, deltaspeed: "
            << Convert::ToString(deltaspeed) << " point: " << Convert::ToString(point);
        LOG_ERROR("PhysicsBody: " + msg.str());
        throw InvalidArgument(msg.str());
    }

    Body->applyImpulse(deltaspeed, point);
}

DLLEXPORT void PhysicsBody::SetVelocity(const Float3& velocities)
{
    if(!Body)
        throw InvalidArgument("PhysicsBody has no longer an internal physics engine body");


    // Safety check. Can be disabled in release builds if this is a performance issue
    if(velocities.HasInvalidValues()) {

        std::stringstream msg;
        msg << "SetVelocity call had at least one value with non-finite value, velocities: "
            << Convert::ToString(velocities);
        LOG_ERROR("PhysicsBody: " + msg.str());
        throw InvalidArgument(msg.str());
    }

    Body->setLinearVelocity(velocities);
}

DLLEXPORT void PhysicsBody::ClearVelocity()
{
    if(!Body)
        throw InvalidArgument("PhysicsBody has no longer an internal physics engine body");


    Body->setAngularVelocity(btVector3(0, 0, 0));
    Body->setLinearVelocity(btVector3(0, 0, 0));
    Body->clearForces();
}

DLLEXPORT Float3 PhysicsBody::GetVelocity() const
{
    if(!Body)
        throw InvalidArgument("PhysicsBody has no longer an internal physics engine body");


    return Body->getLinearVelocity();
}
// ------------------------------------ //
DLLEXPORT Float3 PhysicsBody::GetAngularVelocity() const
{
    if(!Body)
        throw InvalidArgument("PhysicsBody has no longer an internal physics engine body");


    return Body->getAngularVelocity();
}

DLLEXPORT void PhysicsBody::SetAngularVelocity(const Float3& velocities)
{
    if(!Body)
        throw InvalidArgument("PhysicsBody has no longer an internal physics engine body");


    // Safety check. Can be disabled in release builds if this is a performance issue
    if(velocities.HasInvalidValues()) {

        std::stringstream msg;
        msg << "SetAngularVelocity call had at least one value with non-finite value, "
               "velocities: "
            << Convert::ToString(velocities);
        LOG_ERROR("PhysicsBody: " + msg.str());
        throw InvalidArgument(msg.str());
    }

    Body->setAngularVelocity(velocities);
}
// ------------------------------------ //
DLLEXPORT Float3 PhysicsBody::GetTorque() const
{
    if(!Body)
        throw InvalidArgument("PhysicsBody has no longer an internal physics engine body");


    return Body->getTotalTorque();
}

DLLEXPORT void PhysicsBody::ApplyTorque(const Float3& torque)
{
    if(!Body)
        throw InvalidArgument("PhysicsBody has no longer an internal physics engine body");


    // Safety check. Can be disabled in release builds if this is a performance issue
    if(torque.HasInvalidValues()) {

        std::stringstream msg;
        msg << "ApplyTorque call had at least one value with non-finite value, torque: "
            << Convert::ToString(torque);
        LOG_ERROR("PhysicsBody: " + msg.str());
        throw InvalidArgument(msg.str());
    }

    Body->applyTorque(torque);
}

DLLEXPORT void PhysicsBody::SetDamping(float linear, float angular)
{
    if(!Body)
        throw InvalidArgument("PhysicsBody has no longer an internal physics engine body");


    Body->setDamping(linear, angular);
}
// ------------------------------------ //
DLLEXPORT void PhysicsBody::SetMass(float mass)
{
    if(!Body)
        throw InvalidArgument("PhysicsBody has no longer an internal physics engine body");


    // Safety check. Shouldn't be disabled
    if(!std::isfinite(mass)) {

        std::stringstream msg;
        msg << "SetMass mass is invalid value: " << Convert::ToString(mass)
            << " (use 0 for immovable)";
        LOG_ERROR("Physics: " + msg.str());
        throw InvalidArgument(msg.str());
    }

    Mass = mass;

    // First calculate inertia and center of mass points //
    const bool isDynamic = Mass > 0;

    btVector3 localInertia(0, 0, 0);
    if(isDynamic)
        Shape->GetShape()->calculateLocalInertia(Mass, localInertia);

    Body->setMassProps(mass, localInertia);
}
// ------------------------------------ //
DLLEXPORT bool PhysicsBody::CreatePlaneConstraint(
    const Float3& planenormal /*= Float3(0, 1, 0)*/)
{
    if(!Body)
        throw InvalidArgument("PhysicsBody has no longer an internal physics engine body");


    return true;
}
// ------------------------------------ //
DLLEXPORT void PhysicsBody::ApplyMaterial(PhysicalMaterial& material)
{
    if(!Body)
        throw InvalidArgument("PhysicsBody has no longer an internal physics engine body");

    // TODO: properties
}
// ------------------------------------ //
DLLEXPORT void PhysicsPositionProvider::getWorldTransform(btTransform& worldTrans) const
{
    const Float3* position;
    const Float4* orientation;

    GetPositionDataForPhysics(position, orientation);

    worldTrans.setRotation(*orientation);
    worldTrans.setOrigin(*position);
}

DLLEXPORT void PhysicsPositionProvider::setWorldTransform(const btTransform& worldTrans)
{
    const Float3 position = worldTrans.getOrigin();
    const Float4 orientation = worldTrans.getRotation();

    SetPositionDataFromPhysics(position, orientation);
}
