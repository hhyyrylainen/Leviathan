#ifndef LEVIATHAN_SCRIPT_LIST
#define LEVIATHAN_SCRIPT_LIST
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "NamedVars.h"

namespace Leviathan{

	class ScriptList : public Object{
	public:
		DLLEXPORT ScriptList::ScriptList();
		DLLEXPORT ScriptList::ScriptList(wstring name);
		DLLEXPORT ScriptList::~ScriptList();



		wstring Name;
		NamedVars* Variables;
		vector<wstring*> Lines; // for storing plain text //
	

	};

}
#endif