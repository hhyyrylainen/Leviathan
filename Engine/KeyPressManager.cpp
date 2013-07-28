#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_KEYPRESSMANAGER
#include "KeyPressManager.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
// function used for comparing //
bool ComparePtrs(KeyPressManager::RegisteredObject* first, KeyPressManager::RegisteredObject* second){ 
	return (*first) < (*second);
}

KeyPressManager::KeyPressManager() : Receivers(), PendingEvents(){
	Fetched = false;
	//// empty list (is sorted) //
	IsSorted = false;

	// generate id //
	ID = IDFactory::GetID();

	if(StaticInstance){
		// darn //
		Logger::Get()->QueueErrorMessage(L"KeyPressManager: creating another manager, should only be one");
	}
	// set this to be the instance //
	StaticInstance = this;
}
KeyPressManager::~KeyPressManager(){
	// release all listeners //
	SAFE_DELETE_VECTOR(Receivers);
	// should only be one so this is safe //
	StaticInstance = NULL;
}

DLLEXPORT KeyPressManager* Leviathan::KeyPressManager::Get(){
	return StaticInstance;
}

KeyPressManager* Leviathan::KeyPressManager::StaticInstance = NULL;
// ------------------------------------ //
void Leviathan::KeyPressManager::ProcessInput(Input* input){
	if(!Fetched){
		// get all keys //
		//for(int i = 0; i < 256; i++){
		//	PreviousStates[i] = input->GetKeyState(i);
		//}

		Clear();

		Fetched = true;
	}

	if(!IsSorted){
		// needs to sort receivers //
		sort(Receivers.begin(), Receivers.end(), ComparePtrs);
		IsSorted = true;
	}

	// send start event //
	FireEvent(new InputEvent(EVENT_TYPE_EVENT_SEQUENCE_BEGIN, (void*)(new int(ID))));

	// loop through all keys and send events //
	for(int i = 0; i < 256; i++){
		if(input->GetKeyState(i)){
			// key is pressed //

			// send press if wasn't pressed //
			if(!PreviousStates[i]){
				PreviousStates[i] = true;

				// only send press (if something is listening for key down they will receive it next frame) //
				FireEvent(new InputEvent(EVENT_TYPE_KEYPRESS, new int(i)));
				//// also send key down event //
				//FireEvent(new InputEvent(EVENT_TYPE_KEYDOWN, new int(i)));
			} else {
				// send key down event //
				FireEvent(new InputEvent(EVENT_TYPE_KEYDOWN, new int(i)));
			}

		} else {
			PreviousStates[i] = false;
		}
	}

	// mouse events //
	if(input->IsMouseCaptured()){

		int xmoved, ymoved;
		input->GetMouseMoveAmount(xmoved, ymoved);

		// send them //
		FireEvent(new InputEvent(EVENT_TYPE_MOUSEMOVED, (void*)(new Int2(xmoved, ymoved))));

	} else {

		// send mouse relative position //
		int xpos, ypos;
		input->GetMouseRelativeLocation(xpos, ypos);

		// send them (this event should NOT be deleted by anything) //
		FireEvent(new InputEvent(EVENT_TYPE_MOUSEPOSITION, (void*)(new Int2(xpos, ypos))));
	}

	// controller events //


	// empty pending queue allowing events cascade to lower objects //
	ProcessPendingEvents();

	// send end event //
	FireEvent(new InputEvent(EVENT_TYPE_EVENT_SEQUENCE_END, (void*)(new int(ID))));
}

DLLEXPORT void Leviathan::KeyPressManager::Clear(){
	// unset all keys with memset //
	memset(PreviousStates, false, sizeof(PreviousStates));
}

DLLEXPORT bool Leviathan::KeyPressManager::RegisterForEvent(InputReceiver* toregister, int OrderNumber){
	// no longer sorted //
	IsSorted = false;

	// add to vector //
	Receivers.push_back(new RegisteredObject(toregister, OrderNumber));

	return true;
}

DLLEXPORT void Leviathan::KeyPressManager::Unregister(InputReceiver* caller){
	for(size_t i = 0; i < Receivers.size(); i++){
		if(Receivers[i]->callable == caller){
			// remove from pending events (just in case) //
			for(size_t a = 0; a < PendingEvents.size(); a++){
				if(PendingEvents[a]->pendingevent == caller){
					SAFE_DELETE(PendingEvents[a]);
					PendingEvents.erase(PendingEvents.begin()+a);
					a--;
				}
			}

			// delete //
			SAFE_DELETE(Receivers[i]);
			// erase //
			Receivers.erase(Receivers.begin()+i);
			// no longer sorted //
			IsSorted = false;

			// should be only one (don't bother checking for duplicates) //
			return;
		}
	}
}
// ------------------------------------ //
DLLEXPORT int Leviathan::KeyPressManager::FireEvent(InputEvent* eventptr, const int &startorder /*= KEYPRESS_MANAGER_ORDERNUMBER_GUI*/){
	// loop through receivers until event is processed and return order number of receiver which processed this //
	InputEvent** Multiptr = &eventptr;

	bool CanSend = (eventptr->pendingevent == NULL) ? true: false;

	for(size_t i = 0; i < Receivers.size(); i++){
		if(!CanSend){
			// check is this pending //
			if(eventptr->pendingevent == Receivers[i]->callable){
				// no longer pending //
				CanSend = true;
			} else {
				// skip until found the pending one //
				continue;
			}
		}

		if(Receivers[i]->OrderNumber < startorder){
			// may not receive this //
			continue;
		}
		// send //
		bool topending = Receivers[i]->callable->OnEvent(Multiptr, eventptr->pendingevent);

		if(eventptr == NULL){
			// done (event has been deleted) //

			return Receivers[i]->OrderNumber;
		}
		if(topending){
			// add to pending queue, and verify pointer //
			eventptr->pendingevent = Receivers[i]->callable;
			PendingEvents.push_back(eventptr);
			return i;
		} else {
			// clear pending status //
			eventptr->pendingevent = NULL;
		}
	}
	// delete //
	SAFE_DELETE(eventptr);
	// not processed //
	return -1;
}

DLLEXPORT void Leviathan::KeyPressManager::ProcessPendingEvents(){
	while(PendingEvents.size()){

		FireEvent(PendingEvents[0]);

		// fire event will always ensure that the pointer doesn't need to be touched //
		PendingEvents.erase(PendingEvents.begin());
	}
}

// ------------------------------------ //