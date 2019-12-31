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

    DLLEXPORT SceneNode::pointer CreateSceneNode();
    DLLEXPORT void DestroySceneNode(SceneNode::pointer& node);

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
