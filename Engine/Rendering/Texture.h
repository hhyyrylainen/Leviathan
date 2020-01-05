// Leviathan Game Engine
// Copyright (c) 2012-2020 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "Common/ReferenceCounted.h"

#include "bsfCore/BsCorePrerequisites.h"

namespace Leviathan {

class Texture : public ReferenceCounted {
protected:
    // These are protected for only constructing properly reference
    // counted instances through MakeShared
    friend ReferenceCounted;
    DLLEXPORT Texture(const bs::HTexture& texture);

public:
    bs::HTexture GetInternal()
    {
        return BsTexture;
    }

    REFERENCE_COUNTED_PTR_TYPE(Texture);

private:
    bs::HTexture BsTexture;
};

} // namespace Leviathan
