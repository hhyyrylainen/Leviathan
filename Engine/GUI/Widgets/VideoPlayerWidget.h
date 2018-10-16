// Leviathan Game Engine
// Copyright (c) 2012-2018 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "GUI/VideoPlayer.h"
#include "Widget.h"

namespace Leviathan { namespace GUI {

//! \brief Hosts a Leviathan::VideoPlayer in a Widget for diplay in the GUI
class VideoPlayerWidget {
public:
    DLLEXPORT VideoPlayerWidget();
    DLLEXPORT ~VideoPlayerWidget();

private:
    VideoPlayer Player;
};

}} // namespace Leviathan::GUI
