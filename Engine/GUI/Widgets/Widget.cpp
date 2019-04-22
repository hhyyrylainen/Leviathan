// ------------------------------------ //
#include "Widget.h"

#include "Exceptions.h"
#include "Handlers/IDFactory.h"

using namespace Leviathan;
using namespace Leviathan::GUI;
// ------------------------------------ //
DLLEXPORT Widget::Widget() : ID(IDFactory::GetID()) {}
DLLEXPORT Widget::~Widget()
{
    LEVIATHAN_ASSERT(
        ContainedIn == nullptr, "Widget destroyed before being removed from container");
}
// ------------------------------------ //
DLLEXPORT void Widget::OnAddedToContainer(BaseGuiContainer* container)
{
    if(ContainedIn)
        throw InvalidState("Widget added to container while it was in a container");

    ContainedIn = container;
    _AcquireRenderResources();
}

DLLEXPORT void Widget::OnRemovedFromContainer(BaseGuiContainer* container)
{
    if(container != ContainedIn)
        throw InvalidArgument("Widget removed from container that it wasn't in");

    _ReleaseRenderResources();
    ContainedIn = nullptr;
}
