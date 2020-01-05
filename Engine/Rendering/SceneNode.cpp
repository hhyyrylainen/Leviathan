// ------------------------------------ //
#include "SceneNode.h"

#include "Scene.h"

using namespace Leviathan;
// ------------------------------------ //
// SceneAttachable
DLLEXPORT SceneAttachable::~SceneAttachable()
{
    DetachFromParent();
}

DLLEXPORT void SceneAttachable::DetachFromParent()
{
    if(Parent)
        Parent->DetachChild(this);
}
// ------------------------------------ //
DLLEXPORT void SceneAttachable::OnAttachedToParent(SceneNode& parent) {}
DLLEXPORT void SceneAttachable::OnDetachedFromParent(SceneNode& oldparent) {}
// ------------------------------------ //
DLLEXPORT void SceneAttachable::NotifyDetachParent(SceneNode& oldparent)
{
    if(Parent != &oldparent) {
        LOG_ERROR("SceneAttachable: notified detached from wrong parent");
        return;
    }

    Parent = nullptr;
    OnDetachedFromParent(oldparent);
}

DLLEXPORT void SceneAttachable::NotifyAttachParent(SceneNode& parent)
{
    if(Parent) {
        LOG_ERROR(
            "SceneAttachable: notify attach parent called while already attached to a parent");
        DetachFromParent();
    }

    Parent = &parent;
    OnAttachedToParent(parent);
}
// ------------------------------------ //
// SceneNode
DLLEXPORT SceneNode::SceneNode(SceneNode* parent, Scene* scene) :
    Node(bs::SceneObject::create("")), ParentScene(scene)
{
    LEVIATHAN_ASSERT(Node, "bs SceneObject creation failed");

    parent->AttachObject(this);
}


DLLEXPORT SceneNode::SceneNode(bs::HSceneObject node, Scene* scene) :
    Node(node), ParentScene(scene)
{}

DLLEXPORT SceneNode::~SceneNode()
{
    for(auto child : Children)
        child->NotifyDetachParent(*this);
}
// ------------------------------------ //
DLLEXPORT void SceneNode::AttachObject(const SceneAttachable::pointer& object)
{
    if(!object || object.get() == this)
        return;

    // Detach the object automatically if it is already attached
    if(object->HasParent())
        object->DetachFromParent();

    Children.push_back(object);
    object->NotifyAttachParent(*this);
}

DLLEXPORT bool SceneNode::DetachChild(SceneAttachable* child)
{
    if(!child)
        return false;

    for(auto iter = Children.begin(); iter != Children.end(); ++iter) {
        if(*iter == child) {
            (*iter)->NotifyDetachParent(*this);
            Children.erase(iter);
            return true;
        }
    }

    return false;
}
// ------------------------------------ //
DLLEXPORT void SceneNode::OnAttachedToParent(SceneNode& parent)
{
    GetInternal()->setParent(parent.GetInternal(), false);
}

DLLEXPORT void SceneNode::OnDetachedFromParent(SceneNode& oldparent)
{
    if(!ParentScene || !ParentScene->GetRootSceneNode())
        return;

    GetInternal()->setParent(ParentScene->GetRootSceneNode()->GetInternal(), false);
}
