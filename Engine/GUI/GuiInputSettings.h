// Leviathan Game Engine
// Copyright (c) 2012-2018 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //

namespace Leviathan { namespace GUI {

//! This is the mode a View is in
//! \note When mouse capture is enabled the View doesn't get any events until capture mode is
//! disabled
enum class INPUT_MODE {
    //! In this mode the View takes all key press events
    Menu,
    //! In this mode the View will only take key presses when it has an active input box (this
    //! detection is done by a javascript callback that calls LeviathanJavaScriptAsync::OnQuery
    //! to notify the C++ side of the status)
    Gameplay,
    //! In this mode the View doesn't take any input under any circumstances
    None
};

}} // namespace Leviathan::GUI
