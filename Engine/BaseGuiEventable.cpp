#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_GUI_BASEEVENTABLE
#include "BaseGuiEventable.h"
#endif
using namespace Leviathan;
using namespace Leviathan::Gui;
// ------------------------------------ //
#include "GuiManager.h"

DLLEXPORT Leviathan::Gui::BaseGuiEventable::BaseGuiEventable(GuiManager* owner) : InstanceWithRegisteredEvents(owner){

}

BaseGuiEventable::~BaseGuiEventable(){
	InstanceWithRegisteredEvents->RemoveListener(this, EVENT_TYPE_ALL, true);
}


// ------------------------------------ //
void BaseGuiEventable::RegisterForEvent(EVENT_TYPE toregister){
	InstanceWithRegisteredEvents->AddListener(this, toregister);
}
void BaseGuiEventable::UnRegister(EVENT_TYPE from, bool all){
	InstanceWithRegisteredEvents->RemoveListener(this, from);
}
// ------------------------------------ //

// ------------------------------------ //

// ------------------------------------ //