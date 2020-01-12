// ------------------------------------ //
#include "Scene.h"


using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT Scene::Scene()
{
    RootNode = SceneNode::pointer(new SceneNode(this));
}
// ------------------------------------ //
DLLEXPORT SceneNode::pointer Scene::CreateSceneNode()
{
    SceneNode::pointer created(new SceneNode(RootNode.get(), this));

    if(created)
        created->Release();

    // LEVIATHAN_ASSERT(RootNode->HasChild(created.get()), "node not attached to root");

    return created;
}

DLLEXPORT void Scene::DestroySceneNode(SceneNode::pointer& node)
{
    if(!node)
        return;

    node->DetachFromParent();

    node.reset();
}
// ------------------------------------ //
DLLEXPORT void Scene::PrepareForRendering()
{
    RootNode->PrepareToRender();
}
