// Leviathan Game Engine
// Copyright (c) 2012-2020 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "SceneNode.h"

namespace Leviathan {

namespace Rendering {
class Camera;
}

//! \brief Rendering parameters about scene properties
struct RenderingSceneProperties {
public:
    Float3 LightDirection = Float3(0.5f, -0.6f, -0.2f).Normalize();
    Float4 LightColour = Float4(1.f, 1.f, 1.f, 1.f);
    float LightIntensity = 3.f;
};

//! \brief Parameters provided for a renderable to render itself with
struct RenderParams {
public:
    Graphics* _Graphics;
    const Rendering::Camera* _Camera;
    // TODO: viewport

    RenderingSceneProperties SceneProperties;
};

//! \brief Base class for anything that can be rendered and attached to a Scene
class BaseRenderable : public SceneAttachable {
public:
    DLLEXPORT virtual void Render(RenderParams& params) override = 0;
};

} // namespace Leviathan
