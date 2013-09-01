#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_EXCEPTIONINVALIDACCESS
#include "ExceptionInvalidAccess.h"
#endif
using namespace Leviathan;
// ------------------------------------ //

DLLEXPORT Leviathan::ExceptionInvalidAccess::ExceptionInvalidAccess(const wstring &message, int val, const wstring& sourcefunction, 
	const wstring &InvalidArg, const wstring &invalidvalue)  : ExceptionBase(message, val, sourcefunction), InvalidAccessType(new wstring(InvalidArg)),
	InvalidDefinition(new wstring(invalidvalue))
{
	// set type //
	type = EXCEPTIONTYPE_INVALIDACCESS;
}

DLLEXPORT Leviathan::ExceptionInvalidAccess::ExceptionInvalidAccess(const ExceptionInvalidAccess &other){
	// copy this specific value, others should be handled by base class copy ctor //
	this->InvalidAccessType(new wstring(*other.InvalidAccessType));
	this->InvalidDefinition(new wstring(*other.InvalidDefinition));

	// set type //
	type = EXCEPTIONTYPE_INVALIDACCESS;
}



DLLEXPORT Leviathan::ExceptionInvalidAccess::~ExceptionInvalidAccess(){
	// smart pointers auto release //
}
// ------------------------------------ //
DLLEXPORT wstring* Leviathan::ExceptionInvalidAccess::GetInvalidAsPtr(){
	return InvalidAccessType.get();
}

DLLEXPORT wstring Leviathan::ExceptionInvalidAccess::GetInvalid() const{
	return *InvalidAccessType;
}
DLLEXPORT wstring* Leviathan::ExceptionInvalidAccess::GetInvalidMessageAsPtr(){
	return InvalidDefinition.get();
}

DLLEXPORT wstring Leviathan::ExceptionInvalidAccess::GetInvalidMessageAsWstring() const{
	return *InvalidDefinition;
}
// ------------------------------------ //
DLLEXPORT void Leviathan::ExceptionInvalidAccess::PrintToLog() const{
	if(ErrorValue != 0)
		return Logger::Get()->Error(L"[EXCEPTION] InvalidAccess["+*InvalidAccessType+L"; "+*InvalidDefinition+L"] \""+*Message+L"\" from "+*SourceFunction, ErrorValue);
	Logger::Get()->Error(L"[EXCEPTION] InvalidAccess["+*InvalidAccessType+L"; "+*InvalidDefinition+L"] \""+*Message+L"\" from "+*SourceFunction);
}