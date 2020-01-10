// Leviathan Game Engine
// Copyright (c) 2012-2020 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
// part of the hack
#undef LOG_ERROR

#include "DiligentCore/Common/interface/RefCntAutoPtr.h"
#include "DiligentCore/Graphics/GraphicsEngine/interface/SwapChain.h"

// hack workaround
#undef LOG_ERROR
#define LOG_ERROR(x) Logger::Get()->Error(x);

#include <memory>

namespace Leviathan {

class Graphics;

//! \brief Holds a few per-window rendering data
class WindowRenderingResources {
    friend Graphics;

public:
    WindowRenderingResources();

    void ResizeSwapChain(int newwidth, int newheight);

protected:
    Diligent::RefCntAutoPtr<Diligent::ISwapChain> WindowsSwapChain;
};

} // namespace Leviathan
