#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_OBJECTFILE_TEXTBLOCK
#include "ObjectFileTextBlock.h"
#endif
#include "utf8\checked.h"
#include "Exceptions\ExceptionInvalidArgument.h"
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
	// Convert //
	unique_ptr<wstring> converted(new wstring());
	converted->reserve(line.size());

	try{
		utf8::utf8to16(line.begin(), line.end(), back_inserter(*converted));

	} catch(const utf8::invalid_utf8 &e1){

		e1;
		return false;

	} catch(const utf8::not_enough_room &e2){

		e2;
		return false;
	}

	// Conversion succeeded, add //
	Lines.push_back(converted.release());
	return true;
}
// ------------------ ObjectFileTextBlock ------------------ //
DLLEXPORT Leviathan::ObjectFileTextBlock::~ObjectFileTextBlock(){

}
