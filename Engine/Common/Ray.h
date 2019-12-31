// Leviathan Game Engine
// Copyright (c) 2012-2019 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "Types.h"

#include "bsfUtility/Math/BsRay.h"

namespace Leviathan {

struct Ray {
public:
    inline Ray() {}

    inline Ray(const Float3& origin, const Float3& direction) :
        Origin(origin), Direction(direction)
    {}

    inline Ray(const bs::Ray& bsray) :
        Origin(bsray.getOrigin()), Direction(bsray.getDirection())
    {}

    Float3 Origin;
    Float3 Direction;
};

} // namespace Leviathan
