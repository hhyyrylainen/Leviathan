#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_EXCEPTIONINVALIDARGUEMENT
#include "ExceptionInvalidArguement.h"
#endif
using namespace Leviathan;
// ------------------------------------ //

DLLEXPORT Leviathan::ExceptionInvalidArguement::ExceptionInvalidArguement(const wstring &message, int val, const wstring& sourcefunction, 
	const wstring &InvalidArg, const wstring &invalidvalue)  : ExceptionBase(message, val, sourcefunction), InvalidParameter(new wstring(InvalidArg)),
	VisualizedValue(new wstring(invalidvalue))
{
	// set type //
	type = EXCEPTIONTYPE_INVALIDARGUEMENT;
}

DLLEXPORT Leviathan::ExceptionInvalidArguement::ExceptionInvalidArguement(const ExceptionInvalidArguement &other){
	// copy this specific value, others should be handled by base class copy ctor //
	this->InvalidParameter(new wstring(*other.InvalidParameter));
	this->VisualizedValue(new wstring(*other.VisualizedValue));

	// set type //
	type = EXCEPTIONTYPE_INVALIDARGUEMENT;
}



DLLEXPORT Leviathan::ExceptionInvalidArguement::~ExceptionInvalidArguement(){
	// smart pointers auto release //
}
// ------------------------------------ //
DLLEXPORT const wstring* Leviathan::ExceptionInvalidArguement::GetInvalidAsPtr() const{
	return static_cast<const wstring*>(InvalidParameter.get());
}

DLLEXPORT wstring Leviathan::ExceptionInvalidArguement::GetInvalid() const{
	return *InvalidParameter;
}
DLLEXPORT const wstring* Leviathan::ExceptionInvalidArguement::GetInvalidValueAsPtr() const{
	return static_cast<const wstring*>(VisualizedValue.get());
}

DLLEXPORT wstring Leviathan::ExceptionInvalidArguement::GetInvalidAsWstring() const{
	return *VisualizedValue;
}
// ------------------------------------ //
DLLEXPORT void Leviathan::ExceptionInvalidArguement::PrintToLog() const{
	if(ErrorValue != 0)
		return Logger::Get()->Error(L"[EXCEPTION] InvalidArguement ("+*InvalidParameter+L"["+*VisualizedValue+L"]) \""+*Message+L"\" from "+*SourceFunction, ErrorValue);
	Logger::Get()->Error(L"[EXCEPTION] InvalidArguement ("+*InvalidParameter+L"["+*VisualizedValue+L"]) \""+*Message+L"\" from "+*SourceFunction);
}