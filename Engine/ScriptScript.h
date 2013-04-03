#ifndef LEVIATHAN_SCRIPT_SCRIPT
#define LEVIATHAN_SCRIPT_SCRIPT
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "DataBlock.h"
#include "ScriptArguement.h"

#define SCRIPT_RETURN_TYPE_INT		3
#define SCRIPT_RETURN_TYPE_FLOAT	4
#define SCRIPT_RETURN_TYPE_BOOL		5
#define SCRIPT_RETURN_TYPE_WSTRING	6 // is actually wstring
#define SCRIPT_RETURN_TYPE_VOIDPTR	7
//
//#define DATABLOCK_TYPE_INT		3
//#define DATABLOCK_TYPE_FLOAT		4
//#define DATABLOCK_TYPE_BOOL		5
//#define DATABLOCK_TYPE_WSTRING	6 // is actually wstring
//#define DATABLOCK_TYPE_VOIDPTR	7

namespace Leviathan{

	class ScriptScript : public Object{
	public:
		DLLEXPORT ScriptScript::ScriptScript();
		DLLEXPORT ScriptScript::~ScriptScript();

		//int ReturnType;
		wstring Name;
		wstring Source;

		bool Compiled;

		//vector<ScriptNamedArguement*> Parameters;
		//vector<wstring*> Instructions;
		wstring Instructions;
	private:

	};

}
#endif