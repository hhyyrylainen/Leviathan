// Leviathan Game Engine
// Copyright (c) 2012-2018 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "GUI/Widgets/Widget.h"
#include "GuiLayer.h"

namespace Leviathan { namespace GUI {

//! \brief All Leviathan Widget objects need to be contained in a container for rendering
//! \todo This is currenlty hardcoded to work with GuiManager::PlayCutscene if the plan for a
//! custom GUI system is to go forward this needs to be generalized
class WidgetContainer : public Layer {
public:
    DLLEXPORT WidgetContainer(GuiManager* owner, Window* window);
    DLLEXPORT ~WidgetContainer();

    DLLEXPORT void AddWidget(const boost::intrusive_ptr<Widget>& widget);
    DLLEXPORT void RemoveWidget(Widget* widget);


private:
    std::vector<boost::intrusive_ptr<Widget>> Widgets;
};

}} // namespace Leviathan::GUI
