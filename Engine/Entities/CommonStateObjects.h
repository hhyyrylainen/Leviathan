#pragma once
// ------------------------------------ //
#include "Include.h"

#include "Component.h"



namespace Leviathan{

//! \brief State for Physics
class PhysicsComponentState : public ComponentState{
public:

    DLLEXPORT void Interpolate(Physics &target, PhysicsComponentState &other, float progress);

    Member<Float3> Position;
    Member<Float4> Rotation;
    Member<Float3> Velocity;
    Member<Float3> Torque;
};

//! \brief State for Position without a Physics
class PositionComponentState : public ComponentState{
public:

    DLLEXPORT void Interpolate(Position &target, PhysicsComponentState &other, float progress);
    
    Member<Float3> Position;
    Member<Float4> Rotation;
};

}

