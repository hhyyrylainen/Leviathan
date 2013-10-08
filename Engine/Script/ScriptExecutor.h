#ifndef LEVIATHAN_SCRIPT_EXECUTOR
#define LEVIATHAN_SCRIPT_EXECUTOR
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "Script\ScriptScript.h"
#include "Common\DataStoring\DataBlock.h"
#include "angelscript.h"
#include "Script\ScriptRunningSetup.h"

#define ANGELSCRIPT_REGISTERFAIL	Logger::Get()->Error(L"ScriptExecutor: Init: AngelScript: register global failed in file " __WFILE__ L" on line "+Convert::IntToWstring(__LINE__), false);return false;

namespace Leviathan{

	class ScriptModule;

	class ScriptExecutor : public EngineComponent{
		friend ScriptModule;
	public:
		DLLEXPORT ScriptExecutor::ScriptExecutor();
		DLLEXPORT ScriptExecutor::~ScriptExecutor();

		DLLEXPORT bool Init();
		DLLEXPORT void Release();

		DLLEXPORT void ScanAngelScriptTypes();

		// module managing //
		DLLEXPORT weak_ptr<ScriptModule> CreateNewModule(const wstring &name, const string &source, const int &modulesid = IDFactory::GetID());
		DLLEXPORT void DeleteModule(ScriptModule* ptrtomatch); 
		DLLEXPORT weak_ptr<ScriptModule> GetModule(const int &ID);

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
		DLLEXPORT shared_ptr<VariableBlock> RunSetUp(ScriptScript* scriptobject, ScriptRunningSetup* parameters);

	private:
		// ------------------------------ //
		// AngelScript engine script executing part //
		asIScriptEngine* engine;
		// list of modules that have been created, some might only have this as reference, and could potentially be released //
		vector<shared_ptr<ScriptModule>> AllocatedScriptModules;


		// map of type name and engine type id //
		static std::map<int, wstring> EngineTypeIDS;
		// inverted of the former for better performance //
		static std::map<wstring, int> EngineTypeIDSInverted;
	};

}
#endif