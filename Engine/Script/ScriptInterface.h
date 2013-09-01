#ifndef LEVIATHAN_SCRIPT_INTERFACE
#define LEVIATHAN_SCRIPT_INTERFACE
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "Common\DataStoring\DataBlock.h"
#include "Common\DataStoring\NamedVars.h"

#include "Script\ScriptScript.h"
#include "Script\ScriptExecutor.h"
#include "Script\ScriptRunningSetup.h"


namespace Leviathan{

	class ScriptInterface : public EngineComponent{
	public:
		DLLEXPORT ScriptInterface::ScriptInterface();
		DLLEXPORT ScriptInterface::~ScriptInterface();

		DLLEXPORT bool Init();
		DLLEXPORT void Release();

		DLLEXPORT inline shared_ptr<VariableBlock> ExecuteScript(ScriptScript* obj, ScriptRunningSetup* params){
			// just call executor's function //
			return ScriptRunner->RunSetUp(obj, params);
		}

		DLLEXPORT inline ScriptExecutor* GetExecutor(){
			return ScriptRunner;
		}

		DLLEXPORT static ScriptInterface* Get();
	private:

		ScriptExecutor* ScriptRunner;


		// ------------------------------------ //
		static ScriptInterface* StaticAccess;
	};

}
#endif