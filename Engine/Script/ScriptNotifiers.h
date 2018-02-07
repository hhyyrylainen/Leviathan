#pragma once
// ------------------------------------ //
#include "Define.h"
// ------------------------------------ //
#include "Common/BaseNotifiable.h"
#include "angelscript.h"
#include "Common/ReferenceCounted.h"
#include <map>


namespace Leviathan{

	bool RegisterNotifiersWithAngelScript(asIScriptEngine* engine);

	//! \brief BaseNotifier for use in scripts
	class ScriptNotifier : public BaseNotifierAll, public ReferenceCounted{
	public:
		DLLEXPORT ScriptNotifier(asIScriptFunction* functiontocall);
		DLLEXPORT virtual ~ScriptNotifier();


	protected:

		//! \brief Calls into the script engine
		virtual void OnNotified(Lock &ownlock, BaseNotifiableAll* child, Lock &childlock)
            override;

		// ------------------------------------ //

		//! The function that is called when this is notified
		asIScriptFunction* CallbackFunction;
	};


	//! \brief BaseNotifiable for use in scripts
	class ScriptNotifiable : public BaseNotifiableAll, public ReferenceCounted{
	public:
		DLLEXPORT ScriptNotifiable(asIScriptFunction* functiontocall);
		DLLEXPORT virtual ~ScriptNotifiable();


	protected:

		//! \brief Calls into the script engine
		virtual void OnNotified(Lock &ownlock, BaseNotifierAll* parent, Lock &parentlock)
            override;

		// ------------------------------------ //

		//! The function that is called when this is notified
		asIScriptFunction* CallbackFunction;
	};


}

