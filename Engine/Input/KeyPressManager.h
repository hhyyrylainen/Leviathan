#ifndef LEVIATHAN_KEYPRESSMANAGER
#define LEVIATHAN_KEYPRESSMANAGER
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "Input\Input.h"
#include "Events\EventHandler.h"

#define KEYPRESS_MANAGER_ORDERNUMBER_GUI			0
#define KEYPRESS_MANAGER_ORDERNUMBER_ENGINE_END		100
#define KEYPRESS_MANAGER_ORDERNUMBER_LAST_CAMERA	10000

namespace Leviathan{

	class InputReceiver;

	class InputEvent : public Event{
	public:
		DLLEXPORT InputEvent::InputEvent(EVENT_TYPE type, void* data) : Event(type, data), pendingevent(NULL){};
		DLLEXPORT InputEvent::~InputEvent(){};

		// marks events that could not have been processed yet //
		InputReceiver* pendingevent;
	};

	class InputReceiver : public Object{
	public:
		// no virtual destructor because objects are not deleted through this base pointer //
		// virtual function for overloading //
		DLLEXPORT virtual bool OnEvent(InputEvent** pEvent, InputReceiver* pending) = 0;
	};

	class KeyPressManager : public Object{
	public:

		struct RegisteredObject{
			RegisteredObject(InputReceiver* ptr, int order){
				OrderNumber = order;
				callable = ptr;
			}

			// operation for sorting //
			DLLEXPORT inline bool operator < (const RegisteredObject& other) const{
				//// we actually want the smallest order number to be first //
				return OrderNumber < other.OrderNumber;
			}

			int OrderNumber;
			InputReceiver* callable;
		};
		// ------------------------------------ //
		DLLEXPORT KeyPressManager::KeyPressManager();
		DLLEXPORT KeyPressManager::~KeyPressManager();

		DLLEXPORT void ProcessInput(Input* input);
		// clears old state of keys (all keys that were pressed will fire key press event again) //
		DLLEXPORT void Clear();

		DLLEXPORT bool RegisterForEvent(InputReceiver* toregister, int OrderNumber);
		DLLEXPORT void Unregister(InputReceiver* caller);


		// event firing //
		// return value tells which order numbered receiver processed it //
		DLLEXPORT int FireEvent(InputEvent* eventptr, const int &startorder = KEYPRESS_MANAGER_ORDERNUMBER_GUI);
		// clears event queue (should be called after all events have been fired) //
		DLLEXPORT void ProcessPendingEvents();

		// static get //
		DLLEXPORT static KeyPressManager* Get();
	private:
		// static pointer //
		static KeyPressManager* StaticInstance;

		// objects that are called until all events are processed (should also be sorted) //
		vector<RegisteredObject*> Receivers;
		vector<InputEvent*> PendingEvents;

		bool IsSorted : 1;
		bool Fetched : 1;

		int ID;

		// TODO: Add controller support to here //

		bool PreviousStates[256];
	};

}
#endif