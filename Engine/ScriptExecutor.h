#ifndef LEVIATHAN_SCRIPT_EXECUTOR
#define LEVIATHAN_SCRIPT_EXECUTOR
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "ScriptException.h"
#include "ScriptArguement.h"
#include "ScriptCaller.h"
#include "ScriptScript.h"
#include "ScriptArguement.h"
#include "ScriptVariableHolder.h"

#include <angelscript.h>

#define SCRIPT_EXECUTOR_RUNTYPE_BREAKONERROR 1
#define SCRIPT_EXECUTOR_RUNTYPE_TRYTOCONTINUE 2

#define SCRIPT_LINETYPE_FUNCTIONCALL	1
#define SCRIPT_LINETYPE_VARIABLEASSIGN	2
#define SCRIPT_LINETYPE_SEMANTIC		3
#define SCRIPT_LINETYPE_DEFINITION		4
#define SCRIPT_LINETYPE_CONTROLLSTRUCT	5

namespace Leviathan{
	struct ScriptModule{
		ScriptModule();
		~ScriptModule();
		ScriptModule(wstring name, int id);

	private:
		static int LatestAssigned;
	public:
		wstring Name;
		int ID;
		int ModuleID;
	};

	class ScriptExecutor : public EngineComponent{
	public:
		DLLEXPORT ScriptExecutor::ScriptExecutor();
		DLLEXPORT ScriptExecutor::~ScriptExecutor();


		DLLEXPORT bool Init();
		DLLEXPORT bool Release();

		DLLEXPORT shared_ptr<ScriptArguement> RunScript(ScriptScript* script, vector<ScriptCaller*> callers, vector<shared_ptr<ScriptVariableHolder>> gvalues, vector<ScriptNamedArguement*> parameters, bool printerrors, wstring entrance,
			bool ErrorIfdoesnt = true, bool fulldecl = false, int runtype = SCRIPT_EXECUTOR_RUNTYPE_BREAKONERROR);
		DLLEXPORT shared_ptr<ScriptArguement> RunScript(ScriptScript* script, ScriptCaller* caller, shared_ptr<ScriptVariableHolder> gvalues, ScriptNamedArguement* parameter, bool printerrors, wstring entrance,
			bool ErrorIfdoesnt = true, bool fulldecl = false, int runtype = SCRIPT_EXECUTOR_RUNTYPE_BREAKONERROR);

		DLLEXPORT shared_ptr<ScriptArguement> RunSetUp(wstring entrance, bool fulldecl = false, bool ErrorIfdoesnt = true);

		DLLEXPORT void SetScript(ScriptScript* script);
		DLLEXPORT void SetCallers(vector<ScriptCaller*> callers);
		DLLEXPORT void SetGlobalValues(vector<shared_ptr<ScriptVariableHolder>> gvalues);
		DLLEXPORT void SetParameters(vector<ScriptNamedArguement*> parameters);
		DLLEXPORT void SetBehavior(bool printerrors, int runtype = SCRIPT_EXECUTOR_RUNTYPE_BREAKONERROR);
		DLLEXPORT void Clear();

		DLLEXPORT ScriptNamedArguement* GetVariable(wstring& name);
	private:
		shared_ptr<ScriptArguement> RunScript(wstring start, bool fulldecl = false, bool ErrorIfdoesnt = true);
		//int SearchScript(wstring name);
		int CallGlobalFunction(wstring name, wstring unparsedargs);
		asIScriptModule* LoadScript(ScriptScript* script);

		int GetModuleNumber(wstring name, int id = -1);
		int CreateModule(wstring name, int id);


		// ------------------------------ //
		asIScriptEngine *engine;
		vector<ScriptModule*> Modules;

		// ------------------------------ //
		vector<ScriptCaller*> Callbacks;
		vector<shared_ptr<ScriptVariableHolder>> Globals;
		ScriptVariableHolder* ScriptsValues;
		//vector<ScriptScript*> RunningScripts;
		ScriptScript* RunningScripts;
		vector<ScriptNamedArguement*> Parameters;

		vector<ScriptException*> Errors;
		bool PrintErrors;
		int RunType;


	};

}
#endif