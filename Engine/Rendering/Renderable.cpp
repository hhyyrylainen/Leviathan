// ------------------------------------ //
#include "Renderable.h"

#include "Scene.h"

#include "bsfCore/Components/BsCRenderable.h"

using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT Renderable::Renderable(SceneNode& parent)
{
    parent.AttachObject(this);

    LEVIATHAN_ASSERT(GraphicalObject, "GraphicalObject didn't get initialized");
}
// ------------------------------------ //
DLLEXPORT void Renderable::OnAttachedToParent(SceneNode& parent)
{
    GraphicalObject = parent.GetInternal()->addComponent<bs::CRenderable>();
    GraphicalObject->setLayer(1 << parent.GetScene()->GetInternal());
}

DLLEXPORT void Renderable::OnDetachedFromParent(SceneNode& oldparent)
{
    if(GraphicalObject)
        GraphicalObject->destroy();

    GraphicalObject = nullptr;
}
// ------------------------------------ //
DLLEXPORT void Renderable::SetMaterial(const Material::pointer& material)
{
    _Material = material;

    if(!GraphicalObject)
        return;

    if(!_Material || !_Material->GetInternal()) {
        GraphicalObject->setMaterial(nullptr);
    } else {
        GraphicalObject->setMaterial(_Material->GetInternal());
    }
}

DLLEXPORT void Renderable::SetMesh(const Mesh::pointer& mesh)
{
    _Mesh = mesh;

    if(!GraphicalObject)
        return;

    if(!_Mesh || !_Mesh->GetInternal()) {
        GraphicalObject->setMesh(nullptr);
    } else {
        GraphicalObject->setMesh(_Mesh->GetInternal());
    }
}
