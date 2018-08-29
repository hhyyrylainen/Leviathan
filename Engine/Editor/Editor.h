// Leviathan Game Engine
// Copyright (c) 2012-2018 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //

namespace Leviathan {

class Window;

namespace Editor {

//! \brief Main class that handles all editor functionality
class Editor {
public:
    Editor(Window* targetwindow, Engine* engine);
    ~Editor();

protected:
    void _SetupOnWindow(Window* targetwindow);

    //! \brief Closes the editor from the current window. This has to be called before
    //! destroying this or the Window
    void _CloseEditor();

protected:
    //! \todo This needs some mechanism to detect when the window is closed and the editor
    //! should also close then
    Window* ShownOnWindow = nullptr;
};

} // namespace Editor
} // namespace Leviathan
