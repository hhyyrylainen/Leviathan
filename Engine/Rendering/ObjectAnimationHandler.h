// Leviathan Game Engine
// Copyright (c) 2012-2019 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "AnimationTrack.h"
#include "SceneNode.h"

#include "Common/ReferenceCounted.h"

#include "bsfCore/BsCorePrerequisites.h"

namespace Leviathan {

//! \note When not using bsf this will likely be directly placed into a Renderable
class ObjectAnimationHandler : public SceneAttachable {
protected:
    // These are protected for only constructing properly reference
    // counted instances through MakeShared
    friend ReferenceCounted;

    DLLEXPORT ObjectAnimationHandler(SceneNode& parent);

public:
    inline bs::HAnimation GetInternal()
    {
        return Animation;
    }

    REFERENCE_COUNTED_PTR_TYPE(ObjectAnimationHandler);

protected:
    DLLEXPORT virtual void OnAttachedToParent(SceneNode& parent) override;
    DLLEXPORT virtual void OnDetachedFromParent(SceneNode& oldparent) override;

private:
    bs::HAnimation Animation;
};

} // namespace Leviathan
