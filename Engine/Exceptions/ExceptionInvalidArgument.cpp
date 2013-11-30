#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_EXCEPTIONINVALIDARGUEMENT
#include "ExceptionInvalidArgument.h"
#endif
using namespace Leviathan;
// ------------------------------------ //

DLLEXPORT Leviathan::ExceptionInvalidArgument::ExceptionInvalidArgument(const wstring &message, int val, const wstring& sourcefunction, 
	const wstring &InvalidArg, const wstring &invalidvalue)  : ExceptionBase(message, val, sourcefunction), InvalidParameter(new wstring(InvalidArg)),
	VisualizedValue(new wstring(invalidvalue))
{
	// set type //
	type = EXCEPTIONTYPE_INVALIDARGUEMENT;
}

DLLEXPORT Leviathan::ExceptionInvalidArgument::ExceptionInvalidArgument(const ExceptionInvalidArgument &other){
	// copy this specific value, others should be handled by base class copy ctor //
	this->InvalidParameter(new wstring(*other.InvalidParameter));
	this->VisualizedValue(new wstring(*other.VisualizedValue));

	// set type //
	type = EXCEPTIONTYPE_INVALIDARGUEMENT;
}



DLLEXPORT Leviathan::ExceptionInvalidArgument::~ExceptionInvalidArgument(){
	// smart pointers auto release //
}
// ------------------------------------ //
DLLEXPORT const wstring* Leviathan::ExceptionInvalidArgument::GetInvalidAsPtr() const{
	return static_cast<const wstring*>(InvalidParameter.get());
}

DLLEXPORT wstring Leviathan::ExceptionInvalidArgument::GetInvalid() const{
	return *InvalidParameter;
}
DLLEXPORT const wstring* Leviathan::ExceptionInvalidArgument::GetInvalidValueAsPtr() const{
	return static_cast<const wstring*>(VisualizedValue.get());
}

DLLEXPORT wstring Leviathan::ExceptionInvalidArgument::GetInvalidAsWstring() const{
	return *VisualizedValue;
}
// ------------------------------------ //
DLLEXPORT void Leviathan::ExceptionInvalidArgument::PrintToLog() const{
	if(ErrorValue != 0)
		return Logger::Get()->Error(L"[EXCEPTION] InvalidArguement ("+*InvalidParameter+L"["+*VisualizedValue+L"]) \""+*Message+L"\" from "+*SourceFunction, ErrorValue);
	Logger::Get()->Error(L"[EXCEPTION] InvalidArguement ("+*InvalidParameter+L"["+*VisualizedValue+L"]) \""+*Message+L"\" from "+*SourceFunction);
}