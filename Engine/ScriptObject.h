#ifndef LEVIATHAN_SCRIPT_OBJECT
#define LEVIATHAN_SCRIPT_OBJECT
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "ScriptList.h"
#include "ScriptScript.h"
#include "ScriptVariableHolder.h"

namespace Leviathan{

	class ScriptObject : public Object{
	public:
		DLLEXPORT ScriptObject::ScriptObject();
		DLLEXPORT ScriptObject::ScriptObject(wstring name, int basetype, int type, wstring typenam);
		DLLEXPORT ScriptObject::~ScriptObject();



		//DLLEXPORT bool ContainsScript(wstring name); doesn't work anymore

		int BaseType;
		wstring Name;
		wstring TName;
		int Type;

		vector<shared_ptr<wstring>> Prefixes;
		vector<shared_ptr<ScriptList>> Contents;


		//vector<ScriptScript*> Scripts;
		shared_ptr<ScriptScript> Script;
		vector<shared_ptr<ScriptVariableHolder>> Varss;



	};

}
#endif