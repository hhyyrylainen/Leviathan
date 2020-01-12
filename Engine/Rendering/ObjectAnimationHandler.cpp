// ------------------------------------ //
#include "ObjectAnimationHandler.h"

using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT ObjectAnimationHandler::ObjectAnimationHandler(SceneNode& parent)
{
    parent.AttachObject(this);
}
// ------------------------------------ //
DLLEXPORT void ObjectAnimationHandler::OnAttachedToParent(SceneNode& parent)
{
    // Animation = parent.GetInternal()->addComponent<bs::CAnimation>();
}

DLLEXPORT void ObjectAnimationHandler::OnDetachedFromParent(SceneNode& oldparent)
{
    // if(Animation)
    //     Animation->destroy();
    // Animation = nullptr;
}
