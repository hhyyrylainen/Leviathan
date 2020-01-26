// Leviathan Game Engine
// Copyright (c) 2012-2020 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "BaseRenderable.h"
#include "Material.h"
#include "Mesh.h"

namespace Leviathan {

//! \brief Implements a basic renderable rendering a Mesh with a Material
class Renderable : public BaseRenderable {
protected:
    // These are protected for only constructing properly reference
    // counted instances through MakeShared
    friend ReferenceCounted;

    DLLEXPORT Renderable(SceneNode& parent);

public:
    DLLEXPORT void SetMaterial(const Material::pointer& material);
    DLLEXPORT void SetMesh(const Mesh::pointer& mesh);

    DLLEXPORT void Render(RenderParams& params) override;

    REFERENCE_COUNTED_PTR_TYPE(Renderable);

protected:
    DLLEXPORT virtual void OnAttachedToParent(SceneNode& parent) override;
    DLLEXPORT virtual void OnDetachedFromParent(SceneNode& oldparent) override;

private:
    Material::pointer _Material;
    Mesh::pointer _Mesh;
};

} // namespace Leviathan
