// Leviathan Game Engine
// Copyright (c) 2012-2020 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "Types.h"

namespace Leviathan {
struct Plane;
struct Matrix4;

//! \brief Ray starting from an origin with a direction
struct Ray {
public:
    inline Ray() {}

    inline Ray(const Float3& origin, const Float3& direction) :
        Origin(origin), Direction(direction)
    {}

    //! \returns A point along this ray at distance from Origin
    Float3 GetPoint(float distance) const
    {
        return Origin + (Direction * distance);
    }

    DLLEXPORT void Transform(const Matrix4& matrix);

    DLLEXPORT void TransformAffine(const Matrix4& matrix);

    const auto GetOrigin() const
    {
        return Origin;
    }

    const auto GetDirection() const
    {
        return Direction;
    }

    //! \returns Whether this intersects a plane as well as the distance to the intersection
    //! point
    DLLEXPORT std::tuple<bool, float> CalculateIntersection(const Plane& plane) const;

public:
    Float3 Origin;
    Float3 Direction;
};

} // namespace Leviathan

#ifdef LEAK_INTO_GLOBAL
using Leviathan::Ray;
#endif
