// ------------------------------------ //
#include "Widget.h"

#include "Handlers/IDFactory.h"

using namespace Leviathan;
using namespace Leviathan::GUI;
// ------------------------------------ //
DLLEXPORT Widget::Widget() : ID(IDFactory::GetID()) {}
DLLEXPORT Widget::~Widget() {}
// ------------------------------------ //
DLLEXPORT void Widget::OnAddedToContainer(WidgetContainer* container) {}
DLLEXPORT void Widget::OnRemovedFromContainer(WidgetContainer* container) {}
