// ------------------------------------ //
#include "GuiLayer.h"

#include "Handlers/IDFactory.h"

using namespace Leviathan;
using namespace Leviathan::GUI;
// ------------------------------------ //

DLLEXPORT Layer::Layer(GuiManager* owner, Window* window) :
    ID(IDFactory::GetID()), Wind(window), Owner(owner)
{}

DLLEXPORT Layer::~Layer() {}
// ------------------------------------ //
DLLEXPORT void Layer::ReleaseResources() {}
// ------------------------------------ //
DLLEXPORT void Layer::NotifyWindowResized() {}

DLLEXPORT void Layer::NotifyFocusUpdate(bool focused)
{
    OurFocus = focused;
}
// ------------------------------------ //
DLLEXPORT void Layer::SetZVal(float zcoord) {}
