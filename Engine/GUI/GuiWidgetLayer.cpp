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
DLLEXPORT void WidgetLayer::OnRender(float passed)
{
    if(LayoutDirty) {
        OnSizeChanged();
        LayoutDirty = false;
    }
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
    LayoutDirty = true;
    return true;
}

DLLEXPORT bool WidgetLayer::RemoveWidget(Widget* widget)
{
    // For destructing is better to reverse iterate
    for(auto iter = Widgets.rbegin(); iter != Widgets.rend(); ++iter) {

        if(*iter == widget) {

            widget->OnRemovedFromContainer(this);
            Widgets.erase(std::next(iter).base());
            LayoutDirty = true;
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

    LayoutDirty = true;
}
// ------------------------------------ //
DLLEXPORT void WidgetLayer::OnSizeChanged()
{
    // TODO: maybe it would be better that this container would also act like a Panel

    // All widgets added directly to this layer must be able to calculate their own positions
    // (top level widgets)
    for(const auto& widget : Widgets)
        widget->PerformOwnPositioning();
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
