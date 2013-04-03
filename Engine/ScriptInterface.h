#ifndef LEVIATHAN_SCRIPT_INTERFACE
#define LEVIATHAN_SCRIPT_INTERFACE
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "DataBlock.h"
#include "NamedVars.h"

#include "ScriptScript.h"
#include "ScriptObject.h"
#include "ScriptCaller.h"

#include "ScriptExecutor.h"

#define SCRIPT_TYPE_GUI						4000

#define SCRIPT_CALLCONVENTION_GLOBAL		6000
#define SCRIPT_CALLCONVENTION_GUI_OPEN		6001 // has global caller as well //



namespace Leviathan{

	class ScriptInterface : public EngineComponent{
	public:
		DLLEXPORT ScriptInterface::ScriptInterface();
		DLLEXPORT ScriptInterface::~ScriptInterface();


		DLLEXPORT bool Init();
		DLLEXPORT void Release();

		//DLLEXPORT vector<ScriptObject*> ProcessObjectFile(wstring file);

		DLLEXPORT void TakeOwnerShip(ScriptObject* obj); // takes the object as manageable //
		DLLEXPORT void RemoveOwnerShip(ScriptObject* obj, int index = -1);
		DLLEXPORT void ReleaseScript(unsigned int index);
		DLLEXPORT int SearchScript(wstring name, ScriptObject* obj = NULL);
		DLLEXPORT int RunManagedScript(int index);

		DLLEXPORT static ScriptInterface* Get();

		DLLEXPORT shared_ptr<ScriptArguement> ExecuteScript(ScriptObject* obj, wstring entrypoint, vector<ScriptNamedArguement*> Parameters, 
			ScriptCaller* callconv = NULL, bool FullDecl = false);
		DLLEXPORT shared_ptr<ScriptArguement> ExecuteScript(ScriptScript* script, vector<shared_ptr<ScriptVariableHolder>> vars, wstring entrypoint, vector<ScriptNamedArguement*> Parameters, 
			ScriptCaller* callconv = NULL, bool FullDecl = false);
		DLLEXPORT shared_ptr<ScriptArguement> ExecuteIfExistsScript(ScriptObject* obj, wstring entrypoint, vector<ScriptNamedArguement*> Parameters, 
			ScriptCaller* callconv = NULL, bool FullDecl = false);


	private:
		//ScriptObject* ReadObjectBlock(wifstream &reader, wstring firstline, int BaseType, int Type, int &Line, wstring& sourcefile);

		// -------------------- //
		static ScriptInterface* staticaccess;

		ScriptExecutor* ScriptRunner;

		ScriptCaller* GlobalCaller;

		vector<ScriptObject*> Managed;
	};

}
#endif