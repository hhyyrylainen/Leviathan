// ------------------------------------ //
#include "PhysicsConstraint.h"

#include "PhysicsBody.h"

#include "BulletDynamics/ConstraintSolver/btTypedConstraint.h"
using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT PhysicsConstraint::PhysicsConstraint(
    btTypedConstraint* constrainttowrap, PhysicsBody* bodya, PhysicsBody* bodyb) :
    Constraint(constrainttowrap),
    BodyA(bodya), BodyB(bodyb)
{
    LEVIATHAN_ASSERT(Constraint, "PhysicsConstraint given null constraint");

    if(BodyA)
        BodyA->NotifyAttachedConstraint(this);

    if(BodyB)
        BodyB->NotifyAttachedConstraint(this);
}

DLLEXPORT PhysicsConstraint::~PhysicsConstraint()
{
    if(Constraint) {
        LOG_ERROR("PhysicsConstraint: destroyed before being removed from a world");
        DetachResources();
    }
}
// ------------------------------------ //
DLLEXPORT void PhysicsConstraint::DetachResources()
{
    Constraint.reset();

    if(BodyA)
        BodyA->NotifyDetachedConstraint(this);
    BodyA = nullptr;

    if(BodyB)
        BodyB->NotifyDetachedConstraint(this);
    BodyB = nullptr;
}
