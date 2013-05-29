#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_EXCEPTIONBASE
#include "ExceptionBase.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
#include "Logger.h"
DLLEXPORT Leviathan::ExceptionBase::ExceptionBase() : Message(nullptr), SourceFunction(nullptr){

	// set type //
	type = EXCEPTIONTYPE_BASE;
}

DLLEXPORT Leviathan::ExceptionBase::ExceptionBase(const wstring& message, int val, const wstring &source) : Message(new wstring(message)), 
	SourceFunction(new wstring(source))
{
	ErrorValue = val;

	// set type //
	type = EXCEPTIONTYPE_BASE;
}

DLLEXPORT Leviathan::ExceptionBase::ExceptionBase(const ExceptionBase &other){
	this->ErrorValue = other.ErrorValue;
	this->Message(new wstring(*other.Message));
	this->SourceFunction(new wstring(*other.SourceFunction));

	// set type //
	type = EXCEPTIONTYPE_BASE;
}

DLLEXPORT Leviathan::ExceptionBase::~ExceptionBase(){
	// no need to delete (using smart pointer) //

}
// ------------------------------------ //
DLLEXPORT wstring* Leviathan::ExceptionBase::GetMessage(){
	return Message.get();
}

DLLEXPORT wstring Leviathan::ExceptionBase::Get() const{
	return *Message;
}

DLLEXPORT wstring* Leviathan::ExceptionBase::GetSource(){
	return SourceFunction.get();
}

DLLEXPORT wstring Leviathan::ExceptionBase::Source() const{
	return *SourceFunction;
}

DLLEXPORT int Leviathan::ExceptionBase::GetValue() const{
	return ErrorValue;
}
// ------------------------------------ //
//DLLEXPORT void Leviathan::ExceptionBase::PrintToLog(){
//	if(ErrorValue != 0)
//		return Logger::Get()->Error(L"[EXCEPTION] ExceptionBase \""+*Message+L"\" from "+*Source, ErrorValue);
//	Logger::Get()->Error(L"[EXCEPTION] ExceptionBase \""+*Message+L"\" from "+*Source);
//}
// ------------------------------------ //
DLLEXPORT EXCEPTIONTYPE Leviathan::ExceptionBase::GetType() const{
	return type;
}
// ------------------------------------ //






