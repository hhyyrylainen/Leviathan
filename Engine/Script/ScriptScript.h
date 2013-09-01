#ifndef LEVIATHAN_SCRIPT_SCRIPT
#define LEVIATHAN_SCRIPT_SCRIPT
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "ScriptModule.h"

namespace Leviathan{

	// holds a reference of script module //
	class ScriptScript : public Object{
	public:
		DLLEXPORT ScriptScript::ScriptScript(const int &MID, weak_ptr<ScriptModule> wptr);
		DLLEXPORT ScriptScript::ScriptScript(const ScriptScript &other);
		DLLEXPORT ScriptScript::~ScriptScript();


		DLLEXPORT inline ScriptModule* GetModule(){
			return ScriptsModule.lock().get();
		}


	private:
		// reference to module //
		weak_ptr<ScriptModule> ScriptsModule;
		int ModuleID;
	};

}
#endif