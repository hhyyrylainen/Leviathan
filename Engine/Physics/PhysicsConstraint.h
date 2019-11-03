// Leviathan Game Engine
// Copyright (c) 2012-2019 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "Common/ReferenceCounted.h"
#include "Common/Types.h"

#include <memory>

class btTypedConstraint;

namespace Leviathan {

class PhysicsBody;

//! \brief This is an instance of a collision body
class PhysicsConstraint : public ReferenceCounted {
    friend class PhysicalWorld;



protected:
    friend ReferenceCounted;
    DLLEXPORT PhysicsConstraint(
        btTypedConstraint* constrainttowrap, PhysicsBody* bodya, PhysicsBody* bodyb);

public:
    DLLEXPORT ~PhysicsConstraint();

    DLLEXPORT inline bool Valid() const
    {
        return Constraint.operator bool();
    }

    DLLEXPORT inline btTypedConstraint* GetConstraint()
    {
        return Constraint.get();
    }

    REFERENCE_COUNTED_PTR_TYPE(PhysicsConstraint);

protected:
    //! \brief This releases all resources held by this. This is called by the PhysicalWorld
    //! when this is destroyed or the world wants to be destroyed but there are still external
    //! references to this
    DLLEXPORT void DetachResources();

private:
    std::unique_ptr<btTypedConstraint> Constraint;

    // These aren't smart pointers to avoid circular references
    PhysicsBody* BodyA = nullptr;
    PhysicsBody* BodyB = nullptr;
};


} // namespace Leviathan
