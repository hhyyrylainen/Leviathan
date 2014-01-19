#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_EXCEPTIONBASE
#include "Exceptions/ExceptionBase.h"
#endif
using namespace Leviathan;
// ------------------------------------ //

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
	this->Message = unique_ptr<wstring>(new wstring(*other.Message));
	this->SourceFunction = unique_ptr<wstring>(new wstring(*other.SourceFunction));

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
DLLEXPORT EXCEPTIONTYPE Leviathan::ExceptionBase::GetType() const{
	return type;
}
// ------------------------------------ //
DLLEXPORT const char* Leviathan::ExceptionBase::what(){

	ConvertUtility = Convert::WstringToString(*Message.get());

	return ConvertUtility.c_str();
}

//DLLEXPORT const wchar_t* Leviathan::ExceptionBase::what() const{
//	return Message->c_str();
//}





