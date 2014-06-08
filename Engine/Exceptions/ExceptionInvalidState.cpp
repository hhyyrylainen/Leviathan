#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_EXCEPTIONINVALIDSTATE
#include "ExceptionInvalidState.h"
#endif
using namespace Leviathan;
// ------------------------------------ //

DLLEXPORT Leviathan::ExceptionInvalidState::ExceptionInvalidState(const wstring &message, int val, const wstring& sourcefunction,
	const wstring &invalidstate)  : ExceptionBase(message, val, sourcefunction), InvalidStateDescription(new wstring(invalidstate))
{
	// set type //
	type = EXCEPTIONTYPE_INVALIDSTATE;
}

DLLEXPORT Leviathan::ExceptionInvalidState::ExceptionInvalidState(const ExceptionInvalidState &other){
	// copy this specific value, others should be handled by base class copy ctor //
	this->InvalidStateDescription = unique_ptr<wstring>(new wstring(*other.InvalidStateDescription));

	// set type //
	type = EXCEPTIONTYPE_INVALIDSTATE;
}

DLLEXPORT Leviathan::ExceptionInvalidState::~ExceptionInvalidState(){
	// smart pointers auto release //
}
// ------------------------------------ //
DLLEXPORT wstring* Leviathan::ExceptionInvalidState::GetInvalidStateAsPtr(){
	return InvalidStateDescription.get();
}

DLLEXPORT wstring Leviathan::ExceptionInvalidState::GetInvalidStateAsWstring() const{
	return *InvalidStateDescription;
}
// ------------------------------------ //
DLLEXPORT void Leviathan::ExceptionInvalidState::PrintToLog() const{
	if(ErrorValue != 0)
		return Logger::Get()->Error(L"[EXCEPTION] InvalidState["+*InvalidStateDescription+L"] \""+*Message+L"\" from "+*SourceFunction, ErrorValue);
	Logger::Get()->Error(L"[EXCEPTION] InvalidState["+*InvalidStateDescription+L"] \""+*Message+L"\" from "+*SourceFunction);
}
