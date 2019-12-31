// Leviathan Game Engine
// Copyright (c) 2012-2019 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "Common/ReferenceCounted.h"

#include "bsfCore/BsCorePrerequisites.h"

namespace Leviathan {

class AnimationTrack : public ReferenceCounted {
protected:
    // These are protected for only constructing properly reference
    // counted instances through MakeShared
    friend ReferenceCounted;

    DLLEXPORT AnimationTrack(const bs::HAnimationClip& clip);

public:
    REFERENCE_COUNTED_PTR_TYPE(AnimationTrack);

    inline bs::HAnimationClip GetInternal()
    {
        return BsClip;
    }

private:
    bs::HAnimationClip BsClip;
};

} // namespace Leviathan
