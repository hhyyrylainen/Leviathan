// Leviathan Game Engine
// Copyright (c) 2012-2019 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "Common/ReferenceCounted.h"
#include "SceneNode.h"

namespace Leviathan {

class Scene : public ReferenceCounted {
protected:
    // These are protected for only constructing properly reference
    // counted instances through MakeShared
    friend ReferenceCounted;

    DLLEXPORT Scene();

public:
    REFERENCE_COUNTED_PTR_TYPE(Scene);

    //! \brief Creates a new SceneNode as a child of the root node
    DLLEXPORT SceneNode::pointer CreateSceneNode();

    //! \brief "Destroys" a previously created SceneNode
    //!
    //! The node is detached from the scene and its rendering resources are destroyed
    DLLEXPORT void DestroySceneNode(SceneNode::pointer& node);

    DLLEXPORT void Render(RenderParams& params);

    //! \brief Computes the final positions of all SceneNodes to be ready for rendering
    //! \note Called by Render
    DLLEXPORT void PrepareForRendering();

    SceneNode::pointer GetRootSceneNode()
    {
        return RootNode;
    }

    int GetInternal()
    {
        return BsScene;
    }

    SceneNode* GetRootSceneNodeWrapper()
    {
        if(RootNode)
            RootNode->AddRef();
        return RootNode.get();
    }

private:
    int BsScene;

    SceneNode::pointer RootNode;

    //! Workaround hack for no multiple scenes in bsf
    static int BsSceneCounter;
};

} // namespace Leviathan
