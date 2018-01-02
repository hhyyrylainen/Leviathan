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


		DLLEXPORT virtual int OnEvent(Event* event) override;
		DLLEXPORT virtual int OnGenericEvent(GenericEvent* event) override;
		DLLEXPORT virtual bool OnUpdate(const std::shared_ptr<NamedVariableList> &updated)
            override;

	protected:
		// Used by callbacks to call the script for handling //
        //! \todo Allow return values from here
		virtual void _CallScriptListener(Event* event, GenericEvent* event2) = 0;

		// ------------------------------------ //

		// Stores the script reference //
        std::shared_ptr<ScriptScript> Scripting;
	};

}

