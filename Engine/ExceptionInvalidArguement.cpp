#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_EXCEPTIONINVALIDARGUEMENT
#include "ExceptionInvalidArguement.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
#include "Logger.h"

DLLEXPORT Leviathan::ExceptionInvalidArguement::ExceptionInvalidArguement(const wstring &message, int val, const wstring& sourcefunction, 
	const wstring &InvalidArg) : ExceptionBase(message, val, sourcefunction), InvalidParameter(new wstring(InvalidArg))
{
	// set type //
	type = EXCEPTIONTYPE_INVALIDARGUEMENT;
}

DLLEXPORT Leviathan::ExceptionInvalidArguement::ExceptionInvalidArguement(const ExceptionInvalidArguement &other){
	// copy this specific value, others should be handled by base class copy ctor //
	this->InvalidParameter(new wstring(*other.InvalidParameter));

	// set type //
	type = EXCEPTIONTYPE_INVALIDARGUEMENT;
}

DLLEXPORT Leviathan::ExceptionInvalidArguement::~ExceptionInvalidArguement(){
	// smart pointers auto release //
}
// ------------------------------------ //
DLLEXPORT wstring* Leviathan::ExceptionInvalidArguement::GetInvalidAsPtr(){
	return InvalidParameter.get();
}

DLLEXPORT wstring Leviathan::ExceptionInvalidArguement::GetInvalid() const{
	return *InvalidParameter;
}
// ------------------------------------ //
DLLEXPORT void Leviathan::ExceptionInvalidArguement::PrintToLog() const{
	if(ErrorValue != 0)
		return Logger::Get()->Error(L"[EXCEPTION] InvalidArguement ("+*InvalidParameter+L") \""+*Message+L"\" from "+*SourceFunction, ErrorValue);
	Logger::Get()->Error(L"[EXCEPTION] InvalidArguement ("+*InvalidParameter+L") \""+*Message+L"\" from "+*SourceFunction);
}
