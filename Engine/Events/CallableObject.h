#ifndef LEVIATHAN_CALLABLE_OBJECT
#define LEVIATHAN_CALLABLE_OBJECT
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "Event.h"

namespace Leviathan{

	class CallableObject : public Object{
	public:
		DLLEXPORT CallableObject::CallableObject();
		DLLEXPORT virtual CallableObject::~CallableObject();

		DLLEXPORT virtual int OnEvent(Event** pEvent) = 0;
		DLLEXPORT virtual int OnGenericEvent(GenericEvent** pevent) = 0;

		// returns event type from wstring (invalid event type is returned if matches none) //
		// NOTE: if invalid type is returned type should be registered as generic event //
		DLLEXPORT static EVENT_TYPE ResolveStringToType(const wstring &type);


	protected:
		void UnRegister(EVENT_TYPE from, bool all = false);
		void UnRegister(const wstring &genericname, bool all = false);
		void RegisterForEvent(EVENT_TYPE toregister);
		void RegisterForEvent(const wstring &genericname);
		void UnRegisterAllEvents();
		// ------------------------------------ //

		vector<EVENT_TYPE> RegisteredTypes;
		vector<unique_ptr<wstring>> RegisteredGenerics;

	};

}
#endif