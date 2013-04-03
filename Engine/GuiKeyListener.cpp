#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_GUI_KEYLISTENER
#include "GuiKeyListener.h"
#endif
using namespace Leviathan;
using namespace Leviathan::Gui;
// ------------------------------------ //
#include "EventHandler.h"
#include "GuiManager.h"

KeyListener::KeyListener(){
	// register for events //
	EventHandler::Get()->RegisterForEvent(this, EVENT_TYPE_KEYPRESS);
	EventHandler::Get()->RegisterForEvent(this, EVENT_TYPE_KEYDOWN);
}
KeyListener::~KeyListener(){
	// unregister for events //
	EventHandler::Get()->Unregister(this,EVENT_TYPE_KEYPRESS, true); // unregister from all, just incase
}
// ------------------------------------ //
void KeyListener::OnEvent(Event** pEvent){

	if((*pEvent)->Type == EVENT_TYPE_KEYPRESS){
		// put key press into gui keybuffer //
		GuiManager::Get()->AddKeyPress(*(int*)(*pEvent)->Data);
	} else {
		GuiManager::Get()->AddKeyDown(*(int*)(*pEvent)->Data);
	}
}
// ------------------------------------ //

// ------------------------------------ //

// ------------------------------------ //