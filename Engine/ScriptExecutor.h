#ifndef LEVIATHAN_SCRIPT_EXECUTOR
#define LEVIATHAN_SCRIPT_EXECUTOR
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "ScriptScript.h"
#include "DataBlock.h"
#include "angelscript.h"
#include "ScriptRunningSetup.h"

namespace Leviathan{

	class ScriptExecutor : public EngineComponent{
	public:
		DLLEXPORT ScriptExecutor::ScriptExecutor();
		DLLEXPORT ScriptExecutor::~ScriptExecutor();

		DLLEXPORT bool Init();
		DLLEXPORT void Release();

		// module managing //
		DLLEXPORT weak_ptr<ScriptModule> CreateNewModule(const wstring &name, const string &source, const int &modulesid = IDFactory::GetID());
		DLLEXPORT void DeleteModule(ScriptModule* ptrtomatch); 
		DLLEXPORT weak_ptr<ScriptModule> GetModule(const int &ID);

		DLLEXPORT inline asIScriptEngine* GetASEngine(){
			return engine;
		}

		// script running commands //
		DLLEXPORT shared_ptr<VariableBlock> RunSetUp(ScriptScript* scriptobject, ScriptRunningSetup* parameters);

	private:
		// ------------------------------ //
		// AngelScript engine script executing part //
		asIScriptEngine* engine;
		// list of modules that have been created, some might only have this as reference, and could potentially be released //
		vector<shared_ptr<ScriptModule>> AllocatedScriptModules;

	};

}
#endif