#ifndef LEVIATHAN_SCRIPT_CALLER
#define LEVIATHAN_SCRIPT_CALLER
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "ScriptException.h"
#include "ScriptArguement.h"



namespace Leviathan{

	typedef int (*CallerFunctionPointerType)(vector<ScriptArguement*>*);

	class ScriptCaller : public Object{
	public:
		DLLEXPORT ScriptCaller::ScriptCaller();
		DLLEXPORT ScriptCaller::ScriptCaller(bool justrueforglobalcall);
		DLLEXPORT ScriptCaller::~ScriptCaller();

		DLLEXPORT static ScriptCaller* GetGlobal();

		DLLEXPORT int CallFunction(wstring name, vector<ScriptArguement*>* args, int index = -1);

		DLLEXPORT int SearchFunctions(wstring &name);
		DLLEXPORT void RemoveFunction(unsigned int index);
		DLLEXPORT void RegisterFunction(wstring name, CallerFunctionPointerType func);


	private:
		static ScriptCaller* Global;

		vector<CallerFunctionPointerType> Funcptrs;
		vector<wstring*> FunctionNames;
	};

}
#endif