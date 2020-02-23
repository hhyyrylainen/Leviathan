// Leviathan Game Engine
// Copyright (c) 2012-2020 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "BaseRenderable.h"

#include <variant>

namespace Leviathan {

namespace Rendering {
class Model;
}

class Mesh;
class Material;

//! \brief Implements a basic renderable rendering a Mesh with a Material
class Renderable : public BaseRenderable {
protected:
    // These are protected for only constructing properly reference
    // counted instances through MakeShared
    friend ReferenceCounted;

    DLLEXPORT Renderable(SceneNode& parent);

public:
    //! \see SetMesh
    DLLEXPORT void SetMaterial(const CountedPtr<Material>& material);

    //! \note Only a mesh OR a model can be set at a time. Depending on which is used sets some
    //! constraints on what kind of Material can be used.
    DLLEXPORT void SetMesh(const CountedPtr<Mesh>& mesh);

    DLLEXPORT void SetModel(const CountedPtr<Rendering::Model>& model);

    DLLEXPORT void Render(RenderParams& params) override;

    REFERENCE_COUNTED_PTR_TYPE(Renderable);

protected:
    DLLEXPORT virtual void OnAttachedToParent(SceneNode& parent) override;
    DLLEXPORT virtual void OnDetachedFromParent(SceneNode& oldparent) override;

private:
    CountedPtr<Material> _Material;
    std::variant<std::monostate, CountedPtr<Mesh>, CountedPtr<Rendering::Model>> ThingToRender;
};

} // namespace Leviathan
