#pragma once
// ------------------------------------ //
#include "Define.h"
// ------------------------------------ //
#include "Event.h"

namespace Leviathan{

    //! \todo Rewrite this to be cleaner with the event return codes and pointers
	class CallableObject{
	public:
		DLLEXPORT CallableObject();
		DLLEXPORT virtual ~CallableObject();

		DLLEXPORT virtual int OnEvent(Event** pEvent) = 0;
		DLLEXPORT virtual int OnGenericEvent(GenericEvent** pevent) = 0;

		//! \returns event type from wstring (invalid event type is returned if matches none)
		//! \note if invalid type is returned type should be registered as generic event
		DLLEXPORT static EVENT_TYPE ResolveStringToType(const std::string &type);
        
		//! reverse of above and returns for example from EVENT_TYPE_SHOW "OnShow"
		DLLEXPORT static std::string GetListenerNameFromType(EVENT_TYPE type);

	protected:
		void UnRegister(EVENT_TYPE from, bool all = false);
		void UnRegister(const std::string &genericname, bool all = false);
		void RegisterForEvent(EVENT_TYPE toregister);
		void RegisterForEvent(const std::string &genericname);
		void UnRegisterAllEvents();
		// ------------------------------------ //
	};

}

