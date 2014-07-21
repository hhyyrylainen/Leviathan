#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_OBJECTFILE_TEXTBLOCK
#include "ObjectFileTextBlock.h"
#endif
#include "utf8/checked.h"
#include "Exceptions/ExceptionInvalidArgument.h"
using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT Leviathan::ObjectFileTextBlockProper::ObjectFileTextBlockProper(const wstring &name) : Name(name){

}

DLLEXPORT Leviathan::ObjectFileTextBlockProper::~ObjectFileTextBlockProper(){
	SAFE_DELETE_VECTOR(Lines);
}
// ------------------------------------ //
DLLEXPORT const wstring& Leviathan::ObjectFileTextBlockProper::GetName() const{
	return Name;
}
// ------------------------------------ //
DLLEXPORT size_t Leviathan::ObjectFileTextBlockProper::GetLineCount() const{
	return Lines.size();
}

DLLEXPORT const wstring& Leviathan::ObjectFileTextBlockProper::GetLine(size_t index) const THROWS{
	// Check the index //
	if(index >= Lines.size()){

		throw ExceptionInvalidArgument(L"index is out of range", index, __WFUNCTION__, L"index", Convert::ToWstring(index));
	}
	
	return *Lines[index];
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::ObjectFileTextBlockProper::AddTextLine(const string &line){
	
	// Convert and add (if it failed we will have an extra empty line) //
	Lines.push_back(new wstring(Convert::Utf8ToUtf16(line)));
	return true;
}

DLLEXPORT void Leviathan::ObjectFileTextBlockProper::AddTextLine(const wstring &line){
	Lines.push_back(new wstring(line));
}

// ------------------ ObjectFileTextBlock ------------------ //
DLLEXPORT Leviathan::ObjectFileTextBlock::~ObjectFileTextBlock(){

}
