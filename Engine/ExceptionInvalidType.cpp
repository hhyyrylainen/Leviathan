#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_EXCEPTIONINVALIDTYPE
#include "ExceptionInvalidType.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
#include "Logger.h"

DLLEXPORT Leviathan::ExceptionInvalidType::ExceptionInvalidType(const wstring &message, int val, const wstring& sourcefunction, 
	const wstring &invalidvariable, const wstring &invalidtypedescription)  : ExceptionBase(message, val, sourcefunction), 
	InvalidVariable(new wstring(invalidvariable)), InvalidDescription(new wstring(invalidtypedescription))
{
	// set type //
	type = EXCEPTIONTYPE_INVALIDTYPE;
}

DLLEXPORT Leviathan::ExceptionInvalidType::ExceptionInvalidType(const ExceptionInvalidType &other){
	// copy this specific value, others should be handled by base class copy ctor //
	this->InvalidVariable(new wstring(*other.InvalidVariable));
	this->InvalidDescription(new wstring(*other.InvalidDescription));

	// set type //
	type = EXCEPTIONTYPE_INVALIDTYPE;
}

DLLEXPORT Leviathan::ExceptionInvalidType::~ExceptionInvalidType(){
	// smart pointers auto release //
}
// ------------------------------------ //
DLLEXPORT wstring* Leviathan::ExceptionInvalidType::GetInvalidAsPtr(){
	return InvalidVariable.get();
}

DLLEXPORT wstring Leviathan::ExceptionInvalidType::GetInvalid() const{
	return *InvalidVariable;
}
DLLEXPORT wstring* Leviathan::ExceptionInvalidType::GetInvalidDescriptionAsPtr(){
	return InvalidDescription.get();
}

DLLEXPORT wstring Leviathan::ExceptionInvalidType::GetInvalidDescription() const{
	return *InvalidDescription;
}
// ------------------------------------ //
DLLEXPORT void Leviathan::ExceptionInvalidType::PrintToLog() const{
	if(ErrorValue != 0)
		return Logger::Get()->Error(L"[EXCEPTION] InvalidArguement ("+*InvalidVariable+L"<"+*InvalidDescription+L">) \""+*Message+L"\" from "+*SourceFunction, ErrorValue);
	Logger::Get()->Error(L"[EXCEPTION] InvalidArguement ("+*InvalidVariable+L"<"+*InvalidDescription+L">) \""+*Message+L"\" from "+*SourceFunction);
}
