// ------------------------------------ //
#include "Renderable.h"

#include "Camera.h"
#include "Scene.h"

using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT Renderable::Renderable(SceneNode& parent)
{
    parent.AttachObject(this);
}
// ------------------------------------ //
DLLEXPORT void Renderable::OnAttachedToParent(SceneNode& parent) {}

DLLEXPORT void Renderable::OnDetachedFromParent(SceneNode& oldparent) {}
// ------------------------------------ //
DLLEXPORT void Renderable::SetMaterial(const Material::pointer& material)
{
    _Material = material;

    // if(!_Material || !_Material->GetInternal()) {
    //     GraphicalObject->setMaterial(nullptr);
    // } else {
    //     GraphicalObject->setMaterial(_Material->GetInternal());
    // }
}

DLLEXPORT void Renderable::SetMesh(const Mesh::pointer& mesh)
{
    _Mesh = mesh;

    // if(!_Mesh || !_Mesh->GetInternal()) {
    //     GraphicalObject->setMesh(nullptr);
    // } else {
    //     GraphicalObject->setMesh(_Mesh->GetInternal());
    // }
}
// ------------------------------------ //
DLLEXPORT void Renderable::Render(RenderParams& params)
{
    // Skip rendering if no mesh or no material
    if(!_Mesh || !_Material || !HasParent())
        return;

    const auto& transform = GetParent()->GetWorldTransform();

    const auto worldMatrix =
        Matrix4(transform.Translation, transform.Orientation, transform.Scale);

    const auto worldViewProjMatrix =
        (worldMatrix * params._Camera.GetViewMatrix() * params._Camera.GetProjectionMatrix())
            .Transpose();
}
