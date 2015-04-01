#ifndef LEVIATHAN_SCRIPT_EXECUTOR
#define LEVIATHAN_SCRIPT_EXECUTOR
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "Script/ScriptScript.h"
#include "Common/DataStoring/DataBlock.h"

// angelscript //

//#define ANGELSCRIPT_DLL_LIBRARY_IMPORT
#include "angelscript.h"
#include "Script/ScriptRunningSetup.h"
#include "Handlers/IDFactory.h"

#define ANGELSCRIPT_REGISTERFAIL	Logger::Get()->Error("ScriptExecutor: Init: AngelScript: "\
    "register global failed in file " __FILE__ " on line "+Convert::ToString(__LINE__)); \
    return false;

namespace Leviathan{


	class ScriptExecutor : public EngineComponent{
		friend ScriptModule;
	public:
		DLLEXPORT ScriptExecutor();
		DLLEXPORT ~ScriptExecutor();

		DLLEXPORT bool Init();
		DLLEXPORT void Release();

		DLLEXPORT void ScanAngelScriptTypes();

		// module managing //
		DLLEXPORT weak_ptr<ScriptModule> CreateNewModule(const wstring &name, const string &source,
            const int &modulesid = IDFactory::GetID());
        
		DLLEXPORT void DeleteModule(ScriptModule* ptrtomatch);
		DLLEXPORT bool DeleteModuleIfNoExternalReferences(int ID);
		DLLEXPORT weak_ptr<ScriptModule> GetModule(const int &ID);
		DLLEXPORT weak_ptr<ScriptModule> GetModuleByAngelScriptName(const char* nameofmodule);

		DLLEXPORT inline asIScriptEngine* GetASEngine(){
			return engine;
		}

		DLLEXPORT inline int GetAngelScriptTypeID(const wstring &typesname){

			auto iter = EngineTypeIDSInverted.find(typesname);

			if(iter != EngineTypeIDSInverted.end()){
				return iter->second;
			}
			return -1;
		}

		// script running commands //

		//! \brief Runs a script
		DLLEXPORT shared_ptr<VariableBlock> RunSetUp(ScriptScript* scriptobject,
            ScriptRunningSetup* parameters);

		//! \brief Runs a script
		DLLEXPORT shared_ptr<VariableBlock> RunSetUp(ScriptModule* scrptmodule,
            ScriptRunningSetup* parameters);

		//! \brief Runs a script function whose pointer is passed in
		//! \todo Make the module finding more efficient, store module IDs in all call sites
		DLLEXPORT shared_ptr<VariableBlock> RunFunctionSetUp(asIScriptFunction* function,
            ScriptRunningSetup* parameters);

		// Setup functions //


		DLLEXPORT static ScriptExecutor* Get();
	private:

		void PrintAdditionalExcept(asIScriptContext *ctx);

		//! \brief Handles the return type and return value of a function
		shared_ptr<VariableBlock> _GetScriptReturnedVariable(int retcode, asIScriptContext* ScriptContext,
            ScriptRunningSetup* parameters, asIScriptFunction* func, ScriptModule* scrptmodule,
            FunctionParameterInfo* paraminfo);

		//! \brief Handles passing parameters to a context
		bool _SetScriptParameters(asIScriptContext* ScriptContext, ScriptRunningSetup* parameters,
            ScriptModule* scrptmodule, FunctionParameterInfo* paraminfo);

		//! \brief Checks whether a function is a valid pointer
		bool _CheckScriptFunctionPtr(asIScriptFunction* func, ScriptRunningSetup* parameters,
            ScriptModule* scrptmodule);

		//! \brief Prepares a context for usage
		bool _PrepareContextForPassingParameters(asIScriptFunction* func, asIScriptContext* ScriptContext,
            ScriptRunningSetup* parameters, ScriptModule* scrptmodule);

		//! \brief Called when a context is required for script execution
		//! \todo Add a pool from which these are retrieved
		asIScriptContext* _GetContextForExecution();

		//! \brief Called after a script has been executed and the context is no longer needed
		void _DoneWithContext(asIScriptContext* context);

		// ------------------------------ //
		// AngelScript engine script executing part //
		asIScriptEngine* engine;
		// list of modules that have been created, some might only have this as reference, and
        // could potentially be released
		vector<shared_ptr<ScriptModule>> AllocatedScriptModules;


		// map of type name and engine type id //
		static std::map<int, wstring> EngineTypeIDS;
		// inverted of the former for better performance //
		static std::map<wstring, int> EngineTypeIDSInverted;
		static ScriptExecutor* instance;
	};

}
#endif
