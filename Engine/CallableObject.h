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

		DLLEXPORT virtual void OnEvent(Event** pEvent) = 0;


	protected:
		void Leviathan::CallableObject::UnRegister(EVENT_TYPE from, bool all = false);
		void Leviathan::CallableObject::RegisterForEvent(EVENT_TYPE toregister);

	};

}
#endif