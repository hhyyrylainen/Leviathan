// ------------------------------------ //
#include "GuiWidgetContainer.h"

using namespace Leviathan;
using namespace Leviathan::GUI;
// ------------------------------------ //
DLLEXPORT WidgetContainer::WidgetContainer(GuiManager* owner, Window* window) :
    Layer(owner, window)
{}

DLLEXPORT WidgetContainer::~WidgetContainer()
{
    // Remove all widgets
    while(!Widgets.empty()) {

        RemoveWidget(Widgets.back().get());
    }
}
// ------------------------------------ //
DLLEXPORT void WidgetContainer::AddWidget(const boost::intrusive_ptr<Widget>& widget)
{
    // Don't allow duplicates. This is probably quite rare
    for(const auto& widget : Widgets) {

        if(widget == widget) {

            LOG_ERROR("WidgetContainer: AddWidget: trying to add the same widget again");
            return;
        }
    }

    Widgets.push_back(widget);
    widget->OnAddedToContainer(this);
}

DLLEXPORT void WidgetContainer::RemoveWidget(Widget* widget)
{
    // For destructing is better to reverse iterate
    for(auto iter = Widgets.rbegin(); iter != Widgets.rend(); ++iter) {

        if(*iter == widget) {

            widget->OnRemovedFromContainer(this);
            Widgets.erase(std::next(iter).base());
            return;
        }
    }

    LOG_ERROR("WidgetContainer: RemoveWidget: this container has no specified widget");
}
