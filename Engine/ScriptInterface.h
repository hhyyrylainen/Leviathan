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


namespace Leviathan{

	class ScriptInterface : public EngineComponent{
	public:
		DLLEXPORT ScriptInterface::ScriptInterface();
		DLLEXPORT ScriptInterface::~ScriptInterface();


		DLLEXPORT bool Init();
		DLLEXPORT void Release();

		DLLEXPORT void TakeOwnerShip(ScriptObject* obj); // takes the object as manageable //
		DLLEXPORT void RemoveOwnerShip(ScriptObject* obj, size_t index = -1);
		DLLEXPORT void ReleaseScript(size_t index);
		DLLEXPORT int SearchScript(const wstring &name, ScriptObject* obj = NULL);
		DLLEXPORT int RunManagedScript(size_t index);

		DLLEXPORT shared_ptr<ScriptArguement> ExecuteScript(ScriptScript* obj, const wstring &entrypoint, 
			vector<shared_ptr<ScriptNamedArguement>> Parameters, bool FullDecl = false);
		DLLEXPORT shared_ptr<ScriptArguement> ExecuteIfExistsScript(ScriptScript* obj, const wstring &entrypoint, 
			vector<shared_ptr<ScriptNamedArguement>> Parameters, bool &existreceiver, bool FullDecl = false);
		DLLEXPORT shared_ptr<ScriptArguement> ExecuteScript(ScriptObject* obj, const wstring &entrypoint, 
			vector<shared_ptr<ScriptNamedArguement>> Parameters, bool FullDecl = false);
		DLLEXPORT shared_ptr<ScriptArguement> ExecuteIfExistsScript(ScriptObject* obj, const wstring &entrypoint, 
			vector<shared_ptr<ScriptNamedArguement>> Parameters, bool &existreceiver, bool FullDecl = false);

		DLLEXPORT static ScriptInterface* Get();
	private:

		// ------------------------------------ //


		ScriptExecutor* ScriptRunner;

		vector<ScriptObject*> Managed;


		// ------------------------------------ //
		static ScriptInterface* StaticAccess;
	};

}
#endif