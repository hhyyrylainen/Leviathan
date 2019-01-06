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
class WidgetLayer : public Layer {
public:
    DLLEXPORT WidgetLayer(GuiManager* owner, Window* window, int renderorder);
    DLLEXPORT ~WidgetLayer();

    DLLEXPORT void AddWidget(const boost::intrusive_ptr<Widget>& widget);
    DLLEXPORT void RemoveWidget(Widget* widget);

    DLLEXPORT void RemoveAllWidgets();

protected:
    DLLEXPORT void _DoReleaseResources() override;

private:
    std::vector<boost::intrusive_ptr<Widget>> Widgets;
};

}} // namespace Leviathan::GUI
