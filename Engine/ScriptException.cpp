#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_SCRIPT_EXCEPTION
#include "ScriptException.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
ScriptException::ScriptException(){
	ErrorCode = -1;
	ActionValue = -1;
	Message = L"NULL";
	Source = L"NULL";
}
ScriptException::ScriptException(int code, wstring message, wstring source, int actionvalue){
	ErrorCode = code;
	ActionValue = actionvalue;
	Message = message;
	Source = source;
}
ScriptException::~ScriptException(){

}
// ------------------------------------ //