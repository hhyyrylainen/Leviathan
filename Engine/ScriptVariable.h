#ifndef LEVIATHAN_SCRIPT_VARIABLE
#define LEVIATHAN_SCRIPT_VARIABLE
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //

// NON USED ------------------------------------ SAME AS NamedArguement in ScriptArguement.h -------------------------------------------- //

// ---- includes ---- //
#define SCRIPT_VARIABLETTYPE_INT		1
#define SCRIPT_VARIABLETTYPE_FLOAT		2
#define SCRIPT_VARIABLETTYPE_BOOL		3
#define SCRIPT_VARIABLETTYPE_WSTRING	4 // is actually wstring
#define SCRIPT_VARIABLETTYPE_VOIDPTR	5

namespace Leviathan{

	class ScriptVariable : public Object{
	public:
		DLLEXPORT ScriptVariable::ScriptVariable();
        DLLEXPORT ScriptVariable::~ScriptVariable();

		




	private:
		int Type;
		wstring Name;

		int VarType;

		void* Value;

		int iVal;
		float fVal;
		wstring wVal;
	};

}
#endif