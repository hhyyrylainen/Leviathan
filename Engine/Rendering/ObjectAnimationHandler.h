// Leviathan Game Engine
// Copyright (c) 2012-2020 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "AnimationTrack.h"
#include "SceneNode.h"

#include "Common/ReferenceCounted.h"

namespace Leviathan {

//! \note When not using bsf this will likely be directly placed into a Renderable
class ObjectAnimationHandler : public SceneAttachable {
protected:
    // These are protected for only constructing properly reference
    // counted instances through MakeShared
    friend ReferenceCounted;

    DLLEXPORT ObjectAnimationHandler(SceneNode& parent);

public:
    REFERENCE_COUNTED_PTR_TYPE(ObjectAnimationHandler);

protected:
    DLLEXPORT virtual void OnAttachedToParent(SceneNode& parent) override;
    DLLEXPORT virtual void OnDetachedFromParent(SceneNode& oldparent) override;

private:
};

} // namespace Leviathan
