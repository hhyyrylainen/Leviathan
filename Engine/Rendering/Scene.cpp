// ------------------------------------ //
#include "Scene.h"


using namespace Leviathan;
// ------------------------------------ //
int Scene::BsSceneCounter = 0;
// ------------------------------------ //
DLLEXPORT Scene::Scene()
{
    ++BsSceneCounter;

    LEVIATHAN_ASSERT(BsSceneCounter < 32, "ran out of bsf scene bits");

    RootNode = SceneNode::pointer(new SceneNode(bs::SceneObject::create("fake_root"), this));
}
// ------------------------------------ //
DLLEXPORT SceneNode::pointer Scene::CreateSceneNode()
{
    SceneNode::pointer created(new SceneNode(RootNode.get(), this));

    if(created)
        created->Release();

    return created;
}

DLLEXPORT void Scene::DestroySceneNode(SceneNode::pointer& node)
{
    if(!node)
        return;

    if(node->GetInternal())
        node->GetInternal()->destroy();

    node.reset();
}
