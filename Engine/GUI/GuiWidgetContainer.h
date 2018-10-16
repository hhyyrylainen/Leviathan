// Leviathan Game Engine
// Copyright (c) 2012-2018 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "GUI/Widgets/Widget.h"
#include "GuiLayer.h"

namespace Leviathan { namespace GUI {

//! \brief All Leviathan Widget objects need to be contained in a container for rendering
class WidgetContainer : public Layer {
public:
    DLLEXPORT WidgetContainer(GuiManager* owner, Window* window);
    DLLEXPORT ~WidgetContainer();


private:
    std::vector<Widget> Widgets;
};

}} // namespace Leviathan::GUI
