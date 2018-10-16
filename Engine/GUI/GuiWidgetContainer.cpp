// ------------------------------------ //
#include "GuiWidgetContainer.h"

using namespace Leviathan;
using namespace Leviathan::GUI;
// ------------------------------------ //
DLLEXPORT WidgetContainer::WidgetContainer(GuiManager* owner, Window* window) :
    Layer(owner, window)
{}

DLLEXPORT WidgetContainer::~WidgetContainer() {}
