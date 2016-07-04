#pragma once
// ------------------------------------ //
#include "Include.h"

#include "Component.h"

namespace Leviathan{

class StateHelper : public ComponentState{
public:
    StateHelper() = delete;

    
    DLLEXPORT static Float3 Interpolate(Member<Float3> &first,
        Member<Float3> &second, float progress);

    DLLEXPORT static Float4 InterpolateQuoternion(Member<Float4> &first,
        Member<Float4> &second, float progress);
};

//! \brief State for Physics
class PhysicsComponentState : public ComponentState{
public:

    //! \brief Interpolates the target component between this state and the other state
    //!
    //! The other state needs to be the more up to date one and progres HAS to be between
    //! 0.f and 1.f indicating the progress between the states.
    //! \note This may not be missing any values, if this is missing values before the call
    //! this can be filled directly from the component
    DLLEXPORT void Interpolate(Physics &target, PhysicsComponentState &other, float progress);

    Member<Float3> Position;
    Member<Float4> Rotation;
    Member<Float3> Velocity;
    Member<Float3> Torque;
};

//! \brief State for Position without a Physics
class PositionComponentState : public ComponentState{
public:

    //! \copydoc PhysicsComponentState::Interpolate
    DLLEXPORT void Interpolate(Position &target, PositionComponentState &other, float progress);
    
    Member<Float3> _Position;
    Member<Float4> _Rotation;
};

}

