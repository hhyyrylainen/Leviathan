// Leviathan Game Engine
// Copyright (c) 2012-2019 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "Types.h"

namespace Leviathan {
class Ray;

//! \brief A plane represented by a normal and a distance
struct Plane {
public:
    inline Plane(const Float3& normal, float distance) : Normal(normal), Distance(distance) {}

    //! \returns Whether a ray intersects with this as well as the distance to the intersection
    //! from the ray origin
    DLLEXPORT std::tuple<bool, float> CalculateIntersection(const Ray& ray) const;

public:
    Float3 Normal;
    float Distance;
};

} // namespace Leviathan

#ifdef LEAK_INTO_GLOBAL
using Leviathan::Plane;
#endif
