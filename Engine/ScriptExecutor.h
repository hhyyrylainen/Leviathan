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

	// used to store function's parameter info //
	struct FunctionParameterInfo{
		FunctionParameterInfo(int id, int sizes) : FunctionID(id), ParameterTypeIDS(sizes), ParameterDeclarations(sizes), 
			MatchingDataBlockTypes(sizes){};


		int FunctionID;

		vector<asUINT> ParameterTypeIDS;
		vector<string> ParameterDeclarations;
		vector<int> MatchingDataBlockTypes;


		asUINT ReturnTypeID;
		string ReturnTypeDeclaration;
		int ReturnMatchingDataBlock;


	};


	class ScriptModule{
	public:
		ScriptModule(const wstring &name, int id, const wstring &scriptname);
		~ScriptModule();

		FunctionParameterInfo* GetParamInfoForFunction(asIScriptFunction* func); 
		asIScriptModule* GetModule(asIScriptEngine* engine);

	private:
		static int LatestAssigned;
		 // map of type name and engine type id //
		static map<int, string> EngineTypeIDS;
	public:
		wstring Name;
		string ModuleName;
		int ID;
		int ModuleID;


		asIScriptModule* Module;

		vector<FunctionParameterInfo*> FuncParameterInfos;

	private:
		void FillData(int typeofas, asUINT* paramtypeid, string* paramdecl, int* datablocktype);
	};

	class ScriptExecutor : public EngineComponent{
	public:
		DLLEXPORT ScriptExecutor::ScriptExecutor();
		DLLEXPORT ScriptExecutor::~ScriptExecutor();

		DLLEXPORT bool Init();
		DLLEXPORT bool Release();

		// script running commands //
		DLLEXPORT shared_ptr<ScriptArguement> RunScript(ScriptScript* script, vector<shared_ptr<ScriptNamedArguement>> parameters, bool printerrors, 
			const wstring &entrance, bool &existsreceiver, bool ErrorIfdoesnt = true, bool fulldecl = false, int runtype = SCRIPT_EXECUTOR_RUNTYPE_BREAKONERROR);

		DLLEXPORT shared_ptr<ScriptArguement> RunSetUp(const wstring &entrance, bool &existsreceiver, bool fulldecl = false, bool ErrorIfdoesnt = true);

		// data binding commands //
		DLLEXPORT void SetScript(ScriptScript* script);
		DLLEXPORT void SetParameters(vector<shared_ptr<ScriptNamedArguement>> parameters);
		DLLEXPORT void SetBehavior(bool printerrors, int runtype = SCRIPT_EXECUTOR_RUNTYPE_BREAKONERROR);
		DLLEXPORT void Clear();
	private:
		shared_ptr<ScriptArguement> RunScript(const wstring &start, bool &existsreceiver, bool fulldecl = false, bool ErrorIfdoesnt = true);
		int CallGlobalFunction(const wstring &name, const wstring &unparsedargs);

		asIScriptModule* LoadScript(ScriptScript* script, ScriptModule** fetchmodule);

		ScriptModule* GetModule(const wstring &name, int id = -1);
		ScriptModule* CreateModule(const wstring &name, int id, ScriptScript* scrpt);
		// ------------------------------ //
		// AngelScript engine script executing part //
		asIScriptEngine *engine;
		vector<ScriptModule*> Modules;
		// ------------------------------ //
		ScriptScript* RunningScripts;
		vector<shared_ptr<ScriptNamedArguement>> Parameters;


		vector<ScriptException*> Errors;
		bool PrintErrors;
		int RunType;
	};

}
#endif