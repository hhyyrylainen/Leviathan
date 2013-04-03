#ifndef LEVIATHAN_SCRIPT_EXCEPTION
#define LEVIATHAN_SCRIPT_EXCEPTION
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //

namespace Leviathan{

	class ScriptException : public virtual Object{
	public:
		DLLEXPORT ScriptException::ScriptException();
		DLLEXPORT ScriptException::ScriptException(int code, wstring message, wstring source, int actionvalue = -1);
		DLLEXPORT ScriptException::~ScriptException();

		int ErrorCode;
		int ActionValue;
		wstring Message;
		wstring Source;


	};

}
#endif