#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_GUI_BASE_EVENTABLE
#include "EventableGui.h"
#endif
using namespace Leviathan;
using namespace Leviathan::Gui;
// ------------------------------------ //
#include "GuiManager.h"

BaseEventable::BaseEventable(){
	Objecttype = -1;
	HigherLevel = true;
	ObjectLevel = GUI_OBJECT_LEVEL_EVENTABLE;
}
BaseEventable::~BaseEventable(){
	GuiManager::Get()->RemoveListener(this, EVENT_TYPE_ALL, true);
}

//int BaseEventable::OnEvent(Event* pEvent){
//	// return -1 indicating remove this listener //
//	return -1;
//}


		// sort of private //
void BaseEventable::RegisterForEvent(EVENT_TYPE toregister){
	GuiManager::Get()->AddListener(this, toregister);
}
void BaseEventable::UnRegister(EVENT_TYPE from, bool all){
	GuiManager::Get()->RemoveListener(this, from);
}
// ------------------------------------ //

// ------------------------------------ //

// ------------------------------------ //

// ------------------------------------ //