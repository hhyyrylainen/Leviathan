// ------------------------------------ //
#include "GuiWidgetLayer.h"

using namespace Leviathan;
using namespace Leviathan::GUI;
// ------------------------------------ //
DLLEXPORT WidgetLayer::WidgetLayer(GuiManager* owner, Window* window, int renderorder) :
    Layer(owner, window, renderorder)
{}

DLLEXPORT WidgetLayer::~WidgetLayer()
{
    RemoveAllWidgets();
}
// ------------------------------------ //
DLLEXPORT bool WidgetLayer::AddWidget(const boost::intrusive_ptr<Widget>& widget)
{
    // Don't allow duplicates. This is probably quite rare
    for(const auto& existing : Widgets) {

        if(widget == existing) {

            LOG_ERROR("WidgetLayer: AddWidget: trying to add the same widget again");
            return false;
        }
    }

    Widgets.push_back(widget);

    widget->OnAddedToContainer(this);
    return true;
}

DLLEXPORT bool WidgetLayer::RemoveWidget(Widget* widget)
{
    // For destructing is better to reverse iterate
    for(auto iter = Widgets.rbegin(); iter != Widgets.rend(); ++iter) {

        if(*iter == widget) {

            widget->OnRemovedFromContainer(this);
            Widgets.erase(std::next(iter).base());
            return true;
        }
    }

    LOG_ERROR("WidgetLayer: RemoveWidget: this container has no specified widget");
    return false;
}
// ------------------------------------ //
DLLEXPORT void WidgetLayer::RemoveAllWidgets()
{
    // Remove all widgets
    while(!Widgets.empty()) {

        RemoveWidget(Widgets.back().get());
    }
}
// ------------------------------------ //
DLLEXPORT void WidgetLayer::OnSizeChanged()
{
    // TODO: reposition all widgets
}
// ------------------------------------ //
DLLEXPORT void WidgetLayer::_DoReleaseResources()
{
    RemoveAllWidgets();
}

DLLEXPORT void WidgetLayer::_OnWindowResized()
{
    OnSizeChanged();
}

DLLEXPORT void WidgetLayer::_OnFocusChanged() {}
