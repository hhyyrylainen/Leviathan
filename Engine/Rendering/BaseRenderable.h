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

//! \brief Parameters provided for a renderable to render itself with
struct RenderParams {
public:
    Graphics& _Graphics;
    const Rendering::Camera& _Camera;
    // TODO: viewport
};

//! \brief Base class for anything that can be rendered and attached to a Scene
class BaseRenderable : public SceneAttachable {
public:
    //! \brief Called when it is time for this object to render itself
    DLLEXPORT virtual void Render(RenderParams& params) = 0;
};

} // namespace Leviathan
