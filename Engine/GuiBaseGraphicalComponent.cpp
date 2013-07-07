#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_GUIBASEGRAPHICALCOMPONENT
#include "GuiBaseGraphicalComponent.h"
#endif
using namespace Leviathan;
using namespace Leviathan::Gui;
// ------------------------------------ //
DLLEXPORT Leviathan::Gui::BaseGraphicalComponent::BaseGraphicalComponent(int slot, int zorder) : Slot(slot), ZOrder(zorder){
	// set as base type, derived class constructors will override this //
	CType = GUI_GRAPHICALCOMPONENT_TYPE_BASE;
	// set rendering as required //
	RUpdated = true;
}

DLLEXPORT Leviathan::Gui::BaseGraphicalComponent::~BaseGraphicalComponent(){

}
// ------------------------------------ //



