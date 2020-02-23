// ------------------------------------ //
#include "Renderable.h"

#include "Camera.h"
#include "Graphics.h"
#include "Material.h"
#include "Mesh.h"
#include "Model.h"
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
DLLEXPORT void Renderable::SetMaterial(const CountedPtr<Material>& material)
{
    _Material = material;
}

DLLEXPORT void Renderable::SetMesh(const CountedPtr<Mesh>& mesh)
{
    ThingToRender = mesh;
}

DLLEXPORT void Renderable::SetModel(const CountedPtr<Rendering::Model>& model)
{
    ThingToRender = model;
}
// ------------------------------------ //
DLLEXPORT void Renderable::Render(RenderParams& params)
{
    if(!HasParent())
        return;


    if(auto mesh = std::get_if<CountedPtr<Mesh>>(&ThingToRender); mesh) {
        if(!*mesh)
            return;

        // Needs a material
        if(!_Material)
            return;

        const auto& transform = GetParent()->GetWorldTransform();

        const auto worldMatrix =
            Matrix4(transform.Translation, transform.Orientation, transform.Scale);

        const auto worldViewProjMatrix = (worldMatrix * params._Camera->GetViewMatrix() *
                                          params._Camera->GetProjectionMatrix())
                                             .Transpose();

        // TODO: render a plain mesh with a Material
    } else if(auto model = std::get_if<CountedPtr<Rendering::Model>>(&ThingToRender); model) {
        if(!*model)
            return;

        params._Graphics->DrawModel(**model, *GetParent(), params);
    }

    // We don't have any renderable thing set (std::monostate)
}
