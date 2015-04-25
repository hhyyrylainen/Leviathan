#pragma once
// ------------------------------------ //
#include "Define.h"
// ------------------------------------ //
#include "AutoUpdateable.h"
#include "CallableObject.h"
#include "Script/ScriptScript.h"


namespace Leviathan{

	class EventableScriptObject : public AutoUpdateableObject, public CallableObject{
	public:
		DLLEXPORT EventableScriptObject(std::shared_ptr<ScriptScript> script = nullptr);
		DLLEXPORT virtual ~EventableScriptObject();


		DLLEXPORT virtual int OnEvent(Event** pEvent);
		DLLEXPORT virtual int OnGenericEvent(GenericEvent** pevent);
		DLLEXPORT virtual bool OnUpdate(const std::shared_ptr<NamedVariableList> &updated);

	protected:
		// Used by callbacks to call the script for handling //
		virtual void _CallScriptListener(Event** pEvent, GenericEvent** event2) = 0;

		// ------------------------------------ //

		// Stores the script reference //
        std::shared_ptr<ScriptScript> Scripting;
	};

}

