#ifndef LEVIATHAN_SCRIPT_VARIABLEHOLDER
#define LEVIATHAN_SCRIPT_VARIABLEHOLDER
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
//#include "ScriptVariable.h"
#include "ScriptArguement.h"

namespace Leviathan{

	class ScriptVariableHolder : public Object{
	public:
		DLLEXPORT ScriptVariableHolder::ScriptVariableHolder();
        DLLEXPORT ScriptVariableHolder::~ScriptVariableHolder();

		vector<ScriptNamedArguement*> Vars;

	private:

	};

}
#endif