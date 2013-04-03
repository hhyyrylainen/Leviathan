#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_KEYPRESSMANAGER
#include "KeyPressManager.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
KeyPressManager::KeyPressManager(){
	Fetched = false;
}
KeyPressManager::~KeyPressManager(){

}



// ------------------------------------ //
void KeyPressManager::ProcessInput(Input* input){
	if(!Fetched){
		for(int i = 0; i < 256; i++){
			PreviousStates[i] = input->GetKeyPressed(i);
		}
		Fetched = true;
	}

	// loop through all keys and send events //
	for(int i = 0; i < 256; i++){
		if(input->GetKeyPressed(i)){
			// key is pressed //

			// send press if wasn't pressed //
			if(!PreviousStates[i]){
				PreviousStates[i] = TRUE;

				EventHandler::Get()->CallEvent(new Event(EVENT_TYPE_KEYPRESS, new int(i)));
			} else {
				// send keydown event //
				EventHandler::Get()->CallEvent(new Event(EVENT_TYPE_KEYDOWN, new int(i)));

			}

		} else {
			PreviousStates[i] = FALSE;
		}

	}

}
// ------------------------------------ //
void KeyPressManager::Clear(){
	Fetched = false;
}
// ------------------------------------ //

// ------------------------------------ //