// Leviathan Game Engine
// Copyright (c) 2012-2018 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "Common/ReferenceCounted.h"
#include "Common/Types.h"
#include "PhysicsShape.h"

#include "LinearMath/btMotionState.h"

#include <memory>

class btRigidBody;

namespace Leviathan {

class PhysicalMaterial;
class PhysicsDataBridge;

//! \brief This acts as a bridge between Leviathan positions and physics engine positions
//! \todo This should have a property for the object deriving from this to be able to tell that
//! it has destructed
class PhysicsPositionProvider : public btMotionState {
    friend PhysicsDataBridge;

public:
    DLLEXPORT ~PhysicsPositionProvider();

    // These need to be implemented by Leviathan position giver
    virtual void GetPositionDataForPhysics(
        const Float3*& position, const Float4*& orientation) const = 0;
    virtual void SetPositionDataFromPhysics(
        const Float3& position, const Float4& orientation) = 0;

    // These then automatically send the data to the physics engine
    DLLEXPORT void getWorldTransform(btTransform& worldTrans) const override final;
    DLLEXPORT void setWorldTransform(const btTransform& worldTrans) override final;

protected:
    DLLEXPORT void _OnAttachBridge(PhysicsDataBridge* bridge);

private:
    PhysicsDataBridge* AttachedBridge = nullptr;
};

//! \brief This is an indirection helper for allowing world entities to detach from physics
//! objects if they get destroyed
class PhysicsDataBridge : public btMotionState {
    friend PhysicsPositionProvider;

public:
    DLLEXPORT PhysicsDataBridge(PhysicsPositionProvider* provider);
    DLLEXPORT ~PhysicsDataBridge();

    inline auto GetDataSource() const
    {
        return AttachedProvider;
    }

    // These are proxies to the AttachedProvider (otherwise these don't touch the transforms)
    DLLEXPORT void getWorldTransform(btTransform& worldTrans) const override final;
    DLLEXPORT void setWorldTransform(const btTransform& worldTrans) override final;

protected:
    DLLEXPORT void _OnDetachBridge(PhysicsPositionProvider* provider);

private:
    PhysicsPositionProvider* AttachedProvider;
};


//! \brief This is an instance of a collision body
class PhysicsBody : public ReferenceCounted {
    friend class PhysicalWorld;

protected:
    friend ReferenceCounted;
    DLLEXPORT PhysicsBody(std::unique_ptr<btRigidBody>&& body, float mass,
        const PhysicsShape::pointer& shape,
        std::unique_ptr<PhysicsDataBridge>&& positionsynchronization, int materialid);

public:
    DLLEXPORT ~PhysicsBody();

    //! \param point Is relative position of the impulse given
    //! \exception InvalidArgument if deltaspeed or point has non-finite values
    DLLEXPORT void GiveImpulse(const Float3& deltaspeed, const Float3& point = Float3(0));

    //! Overrides this objects velocity in ApplyForceAndTorqueEvent
    DLLEXPORT void SetVelocity(const Float3& velocities);

    //! \brief Clears velocity and last frame forces (but not the applied force list)
    DLLEXPORT void ClearVelocity();

    //! \brief Gets the absolute velocity
    DLLEXPORT Float3 GetVelocity() const;

    //! \brief Gets the omega (angular velocity)
    DLLEXPORT Float3 GetAngularVelocity() const;

    //! \brief Sets the omega
    DLLEXPORT void SetAngularVelocity(const Float3& velocities);

    //! \brief Adds to the torque of this object
    DLLEXPORT void ApplyTorque(const Float3& torque);

    //! \brief Gets the torque of the body (rotational velocity)
    DLLEXPORT Float3 GetTorque() const;

    //! \brief Sets the physical material ID of this object
    //! \note You have to fetch the ID from the world's corresponding PhysicalMaterialManager
    //! \todo There needs to be a physical world helper for actually applying the new
    //! properties
    DLLEXPORT void SetPhysicalMaterialID(int id)
    {
        PhysicalMaterialID = id;
    }

    //! \brief Sets the linear dampening which slows down the object and angular dampening
    //!
    //! Recommended value is 0.1f
    DLLEXPORT void SetDamping(float linear, float angular);

    //! \brief Sets the friction on this body
    DLLEXPORT void SetFriction(float friction);

    //! \brief Moves the physical body to the specified position
    //! \returns False if this fails because there currently is no physics body
    //! for this component
    //! \exception InvalidArgument if pos or orientation has non-finite values
    DLLEXPORT bool SetPosition(const Float3& pos, const Float4& orientation);

    //! \returns The current physical body position
    DLLEXPORT Float3 GetPosition() const;

    //! \brief Same as SetPosition but only sets orientation
    DLLEXPORT bool SetOnlyOrientation(const Float4& orientation);

    // //! \brief Returns the full matrix representing this body's position and rotation
    // DLLEXPORT Ogre::Matrix4 GetFullMatrix() const;

    //! \brief Calculates the mass matrix and applies the mass parameter to the body
    //! \note It is much more effective to calculate the mass first and then make a body
    //! instead of adjusting it later. But if the mass changes then this needs to be called.
    DLLEXPORT void SetMass(float mass);

    DLLEXPORT float GetMass() const
    {
        return Mass;
    }

    //! \brief Adds a constraint to the current Body to only move on specified axises
    //!
    //! This uses the bullet linear factor and replaces
    DLLEXPORT void ConstraintMovementAxises(
        const Float3& movement = Float3(1, 0, 1), const Float3& rotation = Float3(0, 1, 0));

    //! \brief Applies material properties
    //! \todo Add in the properties that make sense as there currently is none
    DLLEXPORT void ApplyMaterial(PhysicalMaterial& material);

    DLLEXPORT inline btRigidBody* GetBody()
    {
        return Body.get();
    }

    inline PhysicsShape* GetShape() const
    {
        return Shape.get();
    }

    inline auto GetPhysicalMaterialID() const
    {
        return PhysicalMaterialID;
    }

    //! Data for helping with callbacks
    DLLEXPORT inline auto GetOwningEntity() const
    {
        return ThisEntity;
    }

    DLLEXPORT inline void SetOwningEntity(ObjectID entity)
    {
        ThisEntity = entity;
    }

    //
    // Script wrappers
    //
    inline PhysicsShape* GetShapeWrapper() const
    {
        if(Shape)
            Shape->AddRef();
        return Shape.get();
    }

    REFERENCE_COUNTED_PTR_TYPE(PhysicsBody);

protected:
    //! \brief This releases all resources held by this. This is called by the PhysicalWorld
    //! when this is destroyed or the world wants to be destroyed but there are still external
    //! references to this
    DLLEXPORT void DetachResources();

    //! \brief Applies shape change
    DLLEXPORT void ApplyShapeChange(const PhysicsShape::pointer& shape);

private:
    std::unique_ptr<btRigidBody> Body;

    //! For access from physics callbacks
    ObjectID ThisEntity = NULL_OBJECT;

    float Mass;

    //! We must keep the shape valid as long as this body exists. Also for recalculating the
    //! mass this is required
    PhysicsShape::pointer Shape;

    //! Hold our side of the bridge open
    std::unique_ptr<PhysicsDataBridge> PositionUpdate;


    int PhysicalMaterialID;
};


} // namespace Leviathan
