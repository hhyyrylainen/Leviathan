#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_EXCEPTIONNULLPTR
#include "ExceptionNULLPtr.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
#include "Logger.h"

DLLEXPORT Leviathan::ExceptionNULLPtr::ExceptionNULLPtr(const wstring& message, int val, void* InvalidPtr) : ExceptionBase(message, val){
	InvalidPointer = InvalidPtr;
	
	// set type //
	type = EXCEPTIONTYPE_NULLPTR;
}
DLLEXPORT Leviathan::ExceptionNULLPtr::ExceptionNULLPtr(const ExceptionNULLPtr &other){
	this->ErrorValue = other.ErrorValue;
	this->Message(new wstring(*other.Message));
	this->InvalidPointer = other.InvalidPointer;

	// set type //
	type = EXCEPTIONTYPE_NULLPTR;
}


DLLEXPORT Leviathan::ExceptionNULLPtr::~ExceptionNULLPtr(){
	// no need to delete (using smart pointer) //

}
// ------------------------------------ //
DLLEXPORT wstring* Leviathan::ExceptionNULLPtr::GetMessage(){
	return Message.get();
}

DLLEXPORT wstring Leviathan::ExceptionNULLPtr::Get(){
	return *Message;
}

DLLEXPORT int Leviathan::ExceptionNULLPtr::GetValue(){
	return ErrorValue;
}

DLLEXPORT void* Leviathan::ExceptionNULLPtr::GetInvalidPtr(){
	return InvalidPointer;
}
// ------------------------------------ //
DLLEXPORT void Leviathan::ExceptionNULLPtr::PrintToLog(){
	if(ErrorValue != 0)
		return Logger::Get()->Error(L"[EXCEPTION] ExceptionNULLPtr \""+*Message+L"\" on pointer "+Convert::ToHexadecimalWstring(InvalidPointer), ErrorValue);
	Logger::Get()->Error(L"[EXCEPTION] ExceptionNULLPtr \""+*Message+L"\" on pointer "+Convert::ToHexadecimalWstring(InvalidPointer));
}
// ------------------------------------ //

// ------------------------------------ //






