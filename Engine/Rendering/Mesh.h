// Leviathan Game Engine
// Copyright (c) 2012-2019 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "Common/ReferenceCounted.h"

#include "bsfCore/BsCorePrerequisites.h"

namespace Leviathan {

class Mesh : public ReferenceCounted {
protected:
    // These are protected for only constructing properly reference
    // counted instances through MakeShared
    friend ReferenceCounted;
    DLLEXPORT Mesh(const bs::HMesh& mesh);

public:
    bs::HMesh GetInternal()
    {
        return BsMesh;
    }

    REFERENCE_COUNTED_PTR_TYPE(Mesh);

private:
    bs::HMesh BsMesh;
};

} // namespace Leviathan
