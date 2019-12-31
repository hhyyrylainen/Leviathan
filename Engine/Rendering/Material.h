// Leviathan Game Engine
// Copyright (c) 2012-2019 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "Common/ReferenceCounted.h"

#include "bsfCore/BsCorePrerequisites.h"

namespace Leviathan {

class Material : public ReferenceCounted {
public:
    bs::HMaterial GetInternal()
    {
        return BsMaterial;
    }

    REFERENCE_COUNTED_PTR_TYPE(Material);

private:
    bs::HMaterial BsMaterial;
};

} // namespace Leviathan
