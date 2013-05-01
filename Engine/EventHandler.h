#ifndef LEVIATHAN_EVENTHANDLER
#define LEVIATHAN_EVENTHANDLER
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "Event.h"
#include "CallableObject.h"

namespace Leviathan{

	struct RegisteredCallback{
		RegisteredCallback(){
			Receiver = NULL;
			Type = EVENT_TYPE_ERROR;
		}
		~RegisteredCallback(){
		}

		RegisteredCallback(CallableObject* receiver, EVENT_TYPE type){
			Receiver = receiver;
			Type = type;
		}
		CallableObject* Receiver;
		EVENT_TYPE Type;
	};

	class EventHandler : public EngineComponent{
	public:
		DLLEXPORT EventHandler::EventHandler();
		DLLEXPORT EventHandler::~EventHandler();

		DLLEXPORT bool Init();
		DLLEXPORT void Release();

		DLLEXPORT static EventHandler* Get();


		DLLEXPORT void CallEvent(Event* pEvent);

		DLLEXPORT bool RegisterForEvent(CallableObject* toregister, EVENT_TYPE totype);
		DLLEXPORT void Unregister(CallableObject* caller, EVENT_TYPE type, bool all = false);

	private:
		vector<RegisteredCallback*> EventListeners;

		static EventHandler* main;

	};

}
#endif