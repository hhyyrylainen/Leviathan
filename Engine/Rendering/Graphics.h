// Leviathan Game Engine
// Copyright (c) 2012-2019 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "Application/AppDefine.h"

#include "bsfCore/RenderAPI/BsRenderWindow.h"

namespace Leviathan {

class Graphics {
    friend Window;

    class Impl;

public:
    DLLEXPORT Graphics();
    DLLEXPORT ~Graphics();

    DLLEXPORT bool Init(AppDef* appdef);
    DLLEXPORT void Release();

    DLLEXPORT bool Frame();

#ifdef __linux
    //! \brief Returns true if our X11 error handler has been called. Remember to check this
    //! after every X11 call
    //!
    //! The value is reset to false after this call
    //! \note This is not thread safe. X11 is also not thread safe so only call on the main
    //! thread
    DLLEXPORT static bool HasX11ErrorOccured();
#endif

protected:
    //! \brief Called when Window objects are created to register them with bsf and with the
    //! case of the first window this initializes the rest of bsf
    bs::SPtr<bs::RenderWindow> RegisterCreatedWindow(Window& window);

private:
    bool InitializeBSF(AppDef* appdef);
    void ShutdownBSF();

private:
    bool Initialized = false;
    bool FirstWindowCreated = false;

    std::unique_ptr<Impl> Pimpl;
};
} // namespace Leviathan
