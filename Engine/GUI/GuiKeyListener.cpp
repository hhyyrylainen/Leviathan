#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_GUI_KEYLISTENER
#include "GuiKeyListener.h"
#endif
using namespace Leviathan;
using namespace Leviathan::Gui;
// ------------------------------------ //
#include "Events\EventHandler.h"
#include "GUI\GuiManager.h"

KeyListener::KeyListener(GuiManager* owner, KeyPressManager* eventsource){
	// store values //
	Master = owner;
	KeySource = eventsource;

	// register //
	KeySource->RegisterForEvent(this, KEYPRESS_MANAGER_ORDERNUMBER_GUI); 
}
KeyListener::~KeyListener(){
	// unregister //
	KeySource->Unregister(this);
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::Gui::KeyListener::OnEvent(InputEvent** pEvent, InputReceiver* pending){
	// check are all events received
	if(pending == this){
		// this is pending //
		// check is it processed //
		if(Master->IsEventConsumed(pEvent)){
			// delete event to indicate that it is processed //
			SAFE_DELETE(*pEvent);
			return false;

		} else {
			// not pending //
			return false;
		}
	}

	// switch on type //
	switch((*pEvent)->Type){
	case EVENT_TYPE_KEYPRESS:
		{
			// add key press to Gui //
			Master->AddKeyPress(*(int*)(*pEvent)->Data, *pEvent);
			// will be pending for a while //
			return true;
		}
		break;
	case EVENT_TYPE_KEYDOWN:
		{
			// add to Gui //
			Master->AddKeyDown(*(int*)(*pEvent)->Data, *pEvent);
			// will be pending for a while //
			return true;
		}
		break;
	case EVENT_TYPE_EVENT_SEQUENCE_BEGIN:
		{
			// clear state for another round for keys //
			Master->ClearKeyReceivingState();
		}
		break;
	}
	// not pending //
	return false;
}
// ------------------------------------ //

// ------------------------------------ //

// ------------------------------------ //