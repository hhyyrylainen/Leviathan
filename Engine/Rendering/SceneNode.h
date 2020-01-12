// Leviathan Game Engine
// Copyright (c) 2012-2020 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "Common/Matrix.h"
#include "Common/Quaternion.h"
#include "Common/ReferenceCounted.h"
#include "Common/Types.h"

namespace Leviathan {

class SceneNode;
class Scene;

//! \brief World transform for an object
//!
//! This is used to keep these properties together when applying parent positioning and
//! transforms
struct Transform {
public:
    Float3 Translation = Float3(0.f, 0.f, 0.f);
    Float3 Scale = Float3(1.f, 1.f, 1.f);
    Quaternion Orientation = Quaternion::IDENTITY;
};


//! \brief Base class for all objects attachable to a SceneNode
class SceneAttachable : public ReferenceCounted {
    friend SceneNode;

public:
    DLLEXPORT ~SceneAttachable();

    //! \brief Detaches this from parent if attached
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
    //! Called by Parent when it becomes dirty to mark all its children to rebuild their
    //! matrices as well
    DLLEXPORT virtual void OnNotifyParentDirty();

    // Methods called by SceneNode
    DLLEXPORT void NotifyDetachParent(SceneNode& oldparent);
    DLLEXPORT void NotifyAttachParent(SceneNode& parent);

    // Called by Scene when it is time to render
    DLLEXPORT virtual void PrepareToRender();

private:
    SceneNode* Parent = nullptr;
};

//! \brief A node in the Scene used to position renderables and other SceneNodes
class SceneNode : public SceneAttachable {
    friend Scene;

protected:
    // These are protected for only constructing properly initialized instances through a Scene
    DLLEXPORT SceneNode(SceneNode* parent, Scene* scene);
    DLLEXPORT SceneNode(Scene* scene);

public:
    //! \note As the scene nodes recursively get detached some care needs to be taken to not
    //! have very deep hierarchies to avoid a stack overflow
    DLLEXPORT ~SceneNode();

    SceneNode(const SceneNode& other) = delete;
    SceneNode operator=(const SceneNode& other) = delete;

    //! \brief Attaches an object to this SceneNode.
    //!
    //! And object can be attached only to one node at a time. This will detach the object
    //! first if it is attached
    DLLEXPORT void AttachObject(const SceneAttachable::pointer& object);

    //! \brief Removes an added child, returns true if succeeded
    DLLEXPORT bool DetachChild(SceneAttachable* child);

    DLLEXPORT bool HasChild(SceneAttachable* child) const;

    inline void SetPosition(const Float3& pos)
    {
        LocalTransform.Translation = pos;
        MarkDirty();
    }

    inline Float3 GetPosition() const
    {
        return LocalTransform.Translation;
    }

    inline void SetOrientation(const Quaternion& orientation)
    {
        LocalTransform.Orientation = orientation;
        MarkDirty();
    }

    //! \brief Alias for SetOrientation
    FORCE_INLINE void SetRotation(const Quaternion& orientation)
    {
        SetOrientation(orientation);
    }

    inline Quaternion GetOrientation() const
    {
        return LocalTransform.Orientation;
    }

    inline void SetPositionAndOrientation(const Float3& pos, const Quaternion& orientation)
    {
        LocalTransform.Translation = pos;
        LocalTransform.Orientation = orientation;
        MarkDirty();
    }

    inline void SetScale(const Float3& scale)
    {
        LocalTransform.Scale = scale;
        MarkDirty();
    }

    void SetHidden(bool hidden)
    {
        if(Hidden == hidden)
            return;

        Hidden = hidden;
    }

    bool IsHidden() const
    {
        return Hidden;
    }

    Scene* GetScene() const
    {
        return ParentScene;
    }

    inline void MarkDirty()
    {
        if(Dirty)
            return;

        Dirty = true;
        TransformDirty = true;
        NotifyChildrenThisIsDirty();
    }

    const Transform& GetWorldTransform() const;

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
    DLLEXPORT void OnNotifyParentDirty() override;

    void ApplyWorldMatrixIfDirty();

    // Called by Scene when it is time to render
    DLLEXPORT void PrepareToRender() override;

    //! Notifies all children that this is now dirty
    DLLEXPORT void NotifyChildrenThisIsDirty() const;

private:
    Scene* ParentScene = nullptr;

    Transform LocalTransform;

    // These computations depend on getting the world transform from the parent
    mutable Transform CachedWorldTransform;
    mutable Matrix4 CachedFinalMatrix = Matrix4::IDENTITY;

    std::vector<SceneAttachable::pointer> Children;

    bool Hidden = false;

    //! true When world transform needs to be updated
    mutable bool Dirty = true;

    //! true when transform has not been applied
    mutable bool TransformDirty = true;
};

} // namespace Leviathan
