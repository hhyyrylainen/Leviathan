#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_EXCEPTIONBASE
#include "ExceptionBase.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
#include "Logger.h"
DLLEXPORT Leviathan::ExceptionBase::ExceptionBase(){

	// set type //
	type = EXCEPTIONTYPE_BASE;
}

DLLEXPORT Leviathan::ExceptionBase::ExceptionBase(const wstring& message, int val) : Message(new wstring(message)){
	ErrorValue = val;

	// set type //
	type = EXCEPTIONTYPE_BASE;
}

DLLEXPORT Leviathan::ExceptionBase::ExceptionBase(const ExceptionBase &other){
	this->ErrorValue = other.ErrorValue;
	this->Message(new wstring(*other.Message));

	// set type //
	type = EXCEPTIONTYPE_NULLPTR;
}

DLLEXPORT Leviathan::ExceptionBase::~ExceptionBase(){
	// no need to delete (using smart pointer) //

}
// ------------------------------------ //
DLLEXPORT wstring* Leviathan::ExceptionBase::GetMessage(){
	return Message.get();
}

DLLEXPORT wstring Leviathan::ExceptionBase::Get(){
	return *Message;
}

DLLEXPORT int Leviathan::ExceptionBase::GetValue(){
	return ErrorValue;
}
// ------------------------------------ //
DLLEXPORT void Leviathan::ExceptionBase::PrintToLog(){
	if(ErrorValue != 0)
		return Logger::Get()->Error(L"[EXCEPTION] ExceptionBase \""+*Message+L"\"", ErrorValue);
	Logger::Get()->Error(L"[EXCEPTION] ExceptionBase \""+*Message+L"\"");
}
// ------------------------------------ //
DLLEXPORT EXCEPTIONTYPE Leviathan::ExceptionBase::GetType(){
	return type;
}
// ------------------------------------ //






