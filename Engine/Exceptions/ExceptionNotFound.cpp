#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_EXCEPTIONNOTFOUND
#include "ExceptionNotFound.h"
#endif
using namespace Leviathan;
// ------------------------------------ //

DLLEXPORT Leviathan::ExceptionNotFound::ExceptionNotFound(const wstring &message, int val, const wstring& sourcefunction, 
	const wstring &InvalidArg, const wstring &invalidvalue)  : ExceptionBase(message, val, sourcefunction), InvalidType(new wstring(InvalidArg)),
	InvalidValue(new wstring(invalidvalue))
{
	// set type //
	type = EXCEPTIONTYPE_NOTFOUND;
}

DLLEXPORT Leviathan::ExceptionNotFound::ExceptionNotFound(const ExceptionNotFound &other){
	// copy this specific value, others should be handled by base class copy ctor //
	this->InvalidType(new wstring(*other.InvalidType));
	this->InvalidValue(new wstring(*other.InvalidValue));

	// set type //
	type = EXCEPTIONTYPE_NOTFOUND;
}

DLLEXPORT Leviathan::ExceptionNotFound::~ExceptionNotFound(){
	// smart pointers auto release //
}
// ------------------------------------ //
DLLEXPORT const wstring* Leviathan::ExceptionNotFound::GetInvalidAsPtr() const{
	return static_cast<const wstring*>(InvalidType.get());
}

DLLEXPORT wstring Leviathan::ExceptionNotFound::GetInvalid() const{
	return *InvalidType;
}
DLLEXPORT const wstring* Leviathan::ExceptionNotFound::GetInvalidValueAsPtr() const{
	return static_cast<const wstring*>(InvalidValue.get());
}

DLLEXPORT wstring Leviathan::ExceptionNotFound::GetInvalidAsWstring() const{
	return *InvalidValue;
}
// ------------------------------------ //
DLLEXPORT void Leviathan::ExceptionNotFound::PrintToLog() const{
	// generate a message //
	const wstring message = L"[EXCEPTION] NotFound ("+*InvalidType+L" \""+*InvalidValue+L"\") \""+*Message+L"\" from "+*SourceFunction;

	if(ErrorValue != 0)
		return Logger::Get()->Error(message, ErrorValue);
	Logger::Get()->Error(message);
}