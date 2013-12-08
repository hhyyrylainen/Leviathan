#ifndef LEVIATHAN_EVENTABLESCRIPTOBJECT
#define LEVIATHAN_EVENTABLESCRIPTOBJECT
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "AutoUpdateable.h"
#include "CallableObject.h"
#include "Script/ScriptScript.h"


namespace Leviathan{

	class EventableScriptObject : public AutoUpdateableObject, public CallableObject{
	public:
		DLLEXPORT EventableScriptObject(shared_ptr<ScriptScript> script = nullptr);
		DLLEXPORT virtual ~EventableScriptObject();


		DLLEXPORT virtual int OnEvent(Event** pEvent);
		DLLEXPORT virtual int OnGenericEvent(GenericEvent** pevent);
		DLLEXPORT virtual bool OnUpdate(const shared_ptr<NamedVariableList> &updated);

	protected:
		// Used by callbacks to call the script for handling //
		virtual void _CallScriptListener(Event** pEvent, GenericEvent** event2) = 0;

		// ------------------------------------ //

		// Stores the script reference //
		shared_ptr<ScriptScript> Scripting;
	};

}
#endif