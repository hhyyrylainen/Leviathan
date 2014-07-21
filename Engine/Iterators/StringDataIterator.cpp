#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_STRINGDATAITERATOR
#include "StringDataIterator.h"
#endif
#include "utf8/checked.h"
using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT Leviathan::StringDataIterator::StringDataIterator() : CurrentCharacterNumber(0), CurrentLineNumber(1){

}

DLLEXPORT Leviathan::StringDataIterator::~StringDataIterator(){

}
// ------------------------------------ //
bool Leviathan::StringDataIterator::ReturnSubString(size_t startpos, size_t endpos, string &receiver){
	Logger::Get()->Error(L"StringDataIterator doesn't support getting with type: string, make sure your provided data source string "
		L"type is the same as the request template type");
	DEBUG_BREAK;
	return false;
}

bool Leviathan::StringDataIterator::ReturnSubString(size_t startpos, size_t endpos, wstring &receiver){
	Logger::Get()->Error(L"StringDataIterator doesn't support getting with type: wstring, make sure your provided data source string "
		L"type is the same as the request template type");
	DEBUG_BREAK;
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
Leviathan::UTF8DataIterator::UTF8DataIterator(const string &str) : OurString(str){
	Current = OurString.begin();
	End = OurString.end();
	BeginPos = OurString.begin();

	int checkchar;

	if(GetNextCharCode(checkchar, 0) && checkchar == '\n'){

		// If the first character is a newline the line number needs to be incremented immediately //
		++CurrentLineNumber;
	}

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


	utf8::advance(shouldbepos, forward, End);

	codepointreceiver = utf8::next(shouldbepos, End);

	return true;
}

bool Leviathan::UTF8DataIterator::GetPreviousCharacter(int &receiver){
	
	// Try to get the prior code point //
	auto shouldbepos = Current;

	try{
		// Try to copy the previous code point into the receiver //
		receiver = utf8::prior(shouldbepos, BeginPos);

	} catch(const utf8::not_enough_room&){
		return false;
	} catch(const utf8::invalid_utf8&){
		return false;
	}

	// If it didn't throw it worked //
	return true;
}
// ------------------------------------ //
void Leviathan::UTF8DataIterator::MoveToNextCharacter(){
	// We need to move whole code points //
	utf8::advance(Current, 1, End);

	// Don't forget to increment these //
	++CurrentCharacterNumber;

	// Return if position is not valid //
	if(!IsPositionValid())
		return;
	// There might be a better way to check this //
	int curcode;
	if(GetNextCharCode(curcode, 0)){
		if(curcode == (int)'\n')
			++CurrentLineNumber;
	}
}
// ------------------------------------ //
size_t Leviathan::UTF8DataIterator::CurrentIteratorPosition() const{
	return std::distance(BeginPos, Current);
}

bool Leviathan::UTF8DataIterator::IsPositionValid() const{
	return Current != End;
}
// ------------------------------------ //
size_t Leviathan::UTF8DataIterator::GetLastValidIteratorPosition() const{
	return OurString.size()-1;
}
// ------------------------------------ //
bool Leviathan::UTF8DataIterator::ReturnSubString(size_t startpos, size_t endpos, string &receiver){
	if(startpos >= OurString.size() || endpos >= OurString.size() || startpos > endpos)
		return false;

	receiver = OurString.substr(startpos, endpos-startpos+1);
	return true;
}
