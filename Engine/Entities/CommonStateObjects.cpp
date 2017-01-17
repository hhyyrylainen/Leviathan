// ------------------------------------ //
#include "CommonStateObjects.h"

#include "Components.h"

#include "Exceptions.h"
using namespace Leviathan;
// ------------------------------------ //

DLLEXPORT Float3 StateHelper::Interpolate(Member<Float3> &first,
    Member<Float3> &second, float progress)
{
    Float3 other = second.Value;
    Float3& full = first.Value;
    
    if(!second.IsBitSet(0)){

        other.X = full.X;
    }

    if(!second.IsBitSet(1)){

        other.Y = full.Y;
    }

    if(!second.IsBitSet(2)){

        other.Z = full.Z;
    }

    return full.Lerp(other, progress);
}

DLLEXPORT Float4 StateHelper::InterpolateQuoternion(Member<Float4> &first,
    Member<Float4> &second, float progress)
{

    Float4 other = second.Value;
    Float4& full = first.Value;
    
    if(!second.IsBitSet(0)){

        other.X = full.X;
    }

    if(!second.IsBitSet(1)){

        other.Y = full.Y;
    }

    if(!second.IsBitSet(2)){

        other.Z = full.Z;
    }

    if(!second.IsBitSet(3)){

        other.W = full.W;
    }

    return full.Slerp(other, progress);
}
// ------------------------------------ //
DLLEXPORT void PhysicsComponentState::Interpolate(Physics &target,
    PhysicsComponentState &other, float progress)
{
    Float3 pos = StateHelper::Interpolate(Position, other.Position, progress);
    Float4 rot = StateHelper::InterpolateQuoternion(Rotation, other.Rotation, progress);
    Float3 vel = StateHelper::Interpolate(Velocity, other.Velocity, progress);
    Float3 tor = StateHelper::Interpolate(Torque, other.Torque, progress);

    target._Position.Members._Position = pos;
    target._Position.Members._Orientation = rot;

    target._Position.Marked = true;
    
    target.SetVelocity(vel);
    target.SetTorque(tor);
}
// ------------------------------------ //
DLLEXPORT void PositionComponentState::Interpolate(Position &target,
    PositionComponentState &other, float progress)
{
    Float3 pos = StateHelper::Interpolate(_Position, other._Position, progress);
    Float4 rot = StateHelper::InterpolateQuoternion(_Rotation, other._Rotation, progress);
    
    target.Members._Position = pos;
    target.Members._Orientation = rot;

    target.Marked = true;    
}
