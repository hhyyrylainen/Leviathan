#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_STRINGDATAITERATOR
#include "StringDataIterator.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT Leviathan::StringDataIterator::StringDataIterator() : CurrentCharacterNumber(0), CurrentLineNumber(1){

}

DLLEXPORT Leviathan::StringDataIterator::~StringDataIterator(){

}
// ------------------------------------ //
bool Leviathan::StringDataIterator::ReturnSubString(size_t startpos, size_t endpos, string &receiver){
	return false;
}

bool Leviathan::StringDataIterator::ReturnSubString(size_t startpos, size_t endpos, wstring &receiver){
	return false;
}
// ------------------------------------ //
size_t Leviathan::StringDataIterator::GetCurrentCharacterNumber() const{
	return CurrentCharacterNumber;
}

size_t Leviathan::StringDataIterator::GetCurrentLineNumber() const{
	return CurrentLineNumber;
}
// ------------------ UTF8DataIterator ------------------ //
Leviathan::UTF8DataIterator::UTF8DataIterator(const string &str) : OurString(str), Current(str.begin()), End(str.end()){

}
// ------------------------------------ //
bool Leviathan::UTF8DataIterator::GetNextCharCode(int &codepointreceiver, size_t forward){
	// We can just peek the next character if forward is 0 //
	if(!forward){

		codepointreceiver = utf8::peek_next(Current, End);
		return true;
	}

	// UTF8 string use the utf8 iterating functions //
	auto shouldbepos = Current;


	for(forward; forward != 0; --forward){

		codepointreceiver = utf8::next(shouldbepos, End);
	}

	return true;
}
// ------------------------------------ //
void Leviathan::UTF8DataIterator::MoveToNextCharacter(){
	++Current;
	// Don't forget to increment these //
	++CurrentCharacterNumber;
	// There might be a better way to check this //
	const int curcode;
	if(GetNextCharCode(curcode, 0)){
		if(curcode == (int)'\n')
			++CurrentLineNumber;
	}
}
// ------------------------------------ //
size_t Leviathan::UTF8DataIterator::CurrentIteratorPosition() const{
	return Current-OurString.begin();
}

bool Leviathan::UTF8DataIterator::IsPositionValid() const{
	return Current != End;
}
