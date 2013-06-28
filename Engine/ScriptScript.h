#ifndef LEVIATHAN_SCRIPT_SCRIPT
#define LEVIATHAN_SCRIPT_SCRIPT
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //


namespace Leviathan{

	class ScriptScript : public Object{
	public:
		DLLEXPORT ScriptScript::ScriptScript();
		DLLEXPORT ScriptScript::~ScriptScript();

		wstring Name;
		wstring Source;

		bool Compiled;

		wstring Instructions;
	private:

	};

}
#endif