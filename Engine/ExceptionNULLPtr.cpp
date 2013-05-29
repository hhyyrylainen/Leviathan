#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_EXCEPTIONNULLPTR
#include "ExceptionNULLPtr.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
#include "Logger.h"

DLLEXPORT Leviathan::ExceptionNULLPtr::ExceptionNULLPtr(const wstring& message, int val, const wstring& sourcefunction, void* InvalidPtr)
	: ExceptionBase(message, val, sourcefunction)
{
	InvalidPointer = InvalidPtr;
	
	// set type //
	type = EXCEPTIONTYPE_NULLPTR;
}
DLLEXPORT Leviathan::ExceptionNULLPtr::ExceptionNULLPtr(const ExceptionNULLPtr &other){
	// just copy the specific value, others should be handled in base class //
	this->InvalidPointer = other.InvalidPointer;

	// set type //
	type = EXCEPTIONTYPE_NULLPTR;
}


DLLEXPORT Leviathan::ExceptionNULLPtr::~ExceptionNULLPtr(){
	// no need to delete (using smart pointer) //

}
// ------------------------------------ //
DLLEXPORT void* Leviathan::ExceptionNULLPtr::GetInvalidPtr() const{
	return InvalidPointer;
}
// ------------------------------------ //
DLLEXPORT void Leviathan::ExceptionNULLPtr::PrintToLog() const{
	if(ErrorValue != 0)
		return Logger::Get()->Error(L"[EXCEPTION] ExceptionNULLPtr \""+*Message+L"\" on pointer "+Convert::ToHexadecimalWstring(InvalidPointer)
		+L" from "+*SourceFunction, ErrorValue);
	Logger::Get()->Error(L"[EXCEPTION] ExceptionNULLPtr \""+*Message+L"\" on pointer "+Convert::ToHexadecimalWstring(InvalidPointer)+L" from "
		+*SourceFunction);
}
// ------------------------------------ //