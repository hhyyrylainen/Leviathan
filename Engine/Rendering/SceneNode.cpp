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
DLLEXPORT void SceneAttachable::PrepareToRender() {}
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
    if(object->HasParent()) {
        if(object->GetParent() == this)
            return;

        object->DetachFromParent();
    }

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
    MarkDirty();
    GetInternal()->setParent(parent.GetInternal(), false);
}

DLLEXPORT void SceneNode::OnDetachedFromParent(SceneNode& oldparent)
{
    if(!ParentScene || !ParentScene->GetRootSceneNode())
        return;

    MarkDirty();

    GetInternal()->setParent(ParentScene->GetRootSceneNode()->GetInternal(), false);
}
// ------------------------------------ //
const Transform& SceneNode::GetWorldTransform() const
{
    if(!Dirty)
        return CachedWorldTransform;

    if(!Parent) {

        CachedWorldTransform = LocalTransform;

        return CachedWorldTransform;
    }

    const auto& parentTransform = Parent->GetWorldTransform();

    // Calculate each 3 Transform member separately
    CachedWorldTransform.Orientation =
        parentTransform.Orientation * LocalTransform.Orientation;

    CachedWorldTransform.Scale = parentTransform.Scale * LocalTransform.Scale;

    // Adjust the local translation based on parent's orientation and scale
    CachedWorldTransform.Translation =
        parentTransform.Orientation * (parentTransform.Scale * LocalTransform.Translation);

    // Add parent position
    CachedWorldTransform.Translation += parentTransform.Translation;

    Dirty = false;
    return CachedWorldTransform;
}
// ------------------------------------ //
void SceneNode::ApplyWorldMatrixIfDirty()
{
    if(!TransformDirty)
        return;

    const auto& transform = GetWorldTransform();

    CachedFinalMatrix =
        Matrix4::FromTRS(transform.Translation, transform.Orientation, transform.Scale);

    // Apply to bsf
    // Can't use the Matrix here, but at least it can be verified that the individual parts are
    // right
    // Node->setPosition(transform.Translation);
    // Node->setRotation(transform.Orientation);
    // Node->setScale(transform.Scale);

    TransformDirty = false;
}

DLLEXPORT void SceneNode::PrepareToRender()
{
    if(TransformDirty) {
        ApplyWorldMatrixIfDirty();
    }

    for(const auto& child : Children) {
        child->PrepareToRender();
    }
}
