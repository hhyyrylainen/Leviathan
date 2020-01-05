// Leviathan Game Engine
// Copyright (c) 2012-2020 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "Common/Quaternion.h"
#include "Common/ReferenceCounted.h"
#include "Common/Types.h"

#include "bsfCore/BsCorePrerequisites.h"
#include "bsfCore/Scene/BsSceneObject.h"

namespace Leviathan {

class SceneNode;
class Scene;

class SceneAttachable : public ReferenceCounted {
    friend SceneNode;

public:
    DLLEXPORT ~SceneAttachable();

    DLLEXPORT void DetachFromParent();

    inline SceneNode* GetParent() const
    {
        return Parent;
    }

    inline bool HasParent() const
    {
        return Parent != nullptr;
    }

    REFERENCE_COUNTED_PTR_TYPE(SceneAttachable);

protected:
    // Callbacks for child classes
    DLLEXPORT virtual void OnAttachedToParent(SceneNode& parent);
    DLLEXPORT virtual void OnDetachedFromParent(SceneNode& oldparent);

    // Methods called by SceneNode
    DLLEXPORT void NotifyDetachParent(SceneNode& oldparent);
    DLLEXPORT void NotifyAttachParent(SceneNode& parent);

private:
    SceneNode* Parent = nullptr;
};

class SceneNode : public SceneAttachable {
    friend Scene;

protected:
    // These are protected for only constructing properly initialized instances through a Scene
    DLLEXPORT SceneNode(SceneNode* parent, Scene* scene);
    DLLEXPORT SceneNode(bs::HSceneObject node, Scene* scene);

public:
    //! \note As the scene nodes recursively get detached some care needs to be taken to not
    //! have very deep hierarchies to avoid a stack overflow
    DLLEXPORT ~SceneNode();

    SceneNode(const SceneNode& other) = delete;
    SceneNode operator=(const SceneNode& other) = delete;

    DLLEXPORT void AttachObject(const SceneAttachable::pointer& object);
    DLLEXPORT bool DetachChild(SceneAttachable* child);

    inline void SetPosition(const Float3& pos)
    {
        Node->setPosition(pos);
    }

    inline Float3 GetPosition() const
    {
        return Node->getLocalTransform().getPosition();
    }

    inline void SetOrientation(const Quaternion& orientation)
    {
        // LOG_WRITE("set orientation = " + Convert::ToString(orientation));
        Node->setRotation(orientation);
    }

    inline Quaternion GetOrientation() const
    {
        return Node->getLocalTransform().getRotation();
    }

    FORCE_INLINE void SetRotation(const Quaternion& rot)
    {
        SetOrientation(rot);
    }

    inline void SetPositionAndOrientation(const Float3& pos, const Quaternion& orientation)
    {
        SetPosition(pos);
        SetOrientation(orientation);
    }

    inline void SetScale(const Float3& scale)
    {
        Node->setScale(scale);
    }

    void SetHidden(bool hidden)
    {
        if(Hidden == hidden)
            return;

        Hidden = hidden;
        Node->setActive(!Hidden);
    }

    Scene* GetScene() const
    {
        return ParentScene;
    }

    inline bs::HSceneObject GetInternal()
    {
        return Node;
    }

    void AttachObjectWrapper(SceneAttachable* object)
    {
        return AttachObject(SceneAttachable::WrapPtr(object));
    }

    void DetachObjectWrapper(SceneAttachable* object)
    {
        // This is used to release the reference given to us from scripts
        const auto wrapped = SceneAttachable::WrapPtr(object);
        return AttachObject(object);
    }

    REFERENCE_COUNTED_PTR_TYPE(SceneNode);

protected:
    DLLEXPORT void OnAttachedToParent(SceneNode& parent) override;
    DLLEXPORT void OnDetachedFromParent(SceneNode& oldparent) override;

private:
    bs::HSceneObject Node;

    Scene* ParentScene = nullptr;

    bool Hidden = false;

    std::vector<SceneAttachable::pointer> Children;
};

} // namespace Leviathan
