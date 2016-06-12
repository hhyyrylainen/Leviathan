#include "Include.h"
// ------------------------------------ //
#include "StringIterator.h"

#include "utf8/checked.h"
using namespace Leviathan;
using namespace std;
// ------------------------------------ //
DLLEXPORT StringIterator::StringIterator() :
    CurrentFlags(0), HandlesDelete(false), DataIterator(NULL), CurrentStored(false)
{

}

DLLEXPORT Leviathan::StringIterator::StringIterator(StringDataIterator* iterator, bool TakesOwnership) : CurrentFlags(0), 
	HandlesDelete(TakesOwnership), DataIterator(iterator), CurrentStored(false), CurrentCharacter(-1)
{

}

DLLEXPORT Leviathan::StringIterator::StringIterator(const string &text) : CurrentFlags(0), HandlesDelete(true), 
	DataIterator(new StringClassDataIterator<string>(text)), CurrentStored(false), CurrentCharacter(-1)
{

}

DLLEXPORT Leviathan::StringIterator::StringIterator(const wstring &text) : CurrentFlags(0), HandlesDelete(true), 
	DataIterator(new StringClassDataIterator<wstring>(text)), CurrentStored(false), CurrentCharacter(-1)
{

}

DLLEXPORT Leviathan::StringIterator::StringIterator(const wstring* text) : CurrentFlags(0), HandlesDelete(true), 
	DataIterator(new StringClassPointerIterator<wstring>(text)), CurrentStored(false), CurrentCharacter(-1){

}

DLLEXPORT Leviathan::StringIterator::StringIterator(const string* text) : CurrentFlags(0), HandlesDelete(true), 
	DataIterator(new StringClassPointerIterator<string>(text)), CurrentStored(false), CurrentCharacter(-1){

}

DLLEXPORT Leviathan::StringIterator::~StringIterator(){
	if(HandlesDelete){

		SAFE_DELETE(DataIterator);
	}
}
// ------------------------------------ //
DLLEXPORT void Leviathan::StringIterator::ReInit(StringDataIterator* iterator,
    bool TakesOwnership /*= false*/)
{
	// Remove the last iterator //
	if(HandlesDelete){

		SAFE_DELETE(DataIterator);
	}

    ITR_COREDEBUG("ReInit")

	HandlesDelete = TakesOwnership;
	DataIterator = iterator;

	// Reset everything //
	CurrentCharacter = -1;
	CurrentStored = false;


	// Clear the flags //
	CurrentFlags = 0;
}

DLLEXPORT void Leviathan::StringIterator::ReInit(const wstring &text){
	ReInit(new StringClassDataIterator<wstring>(text), true);
}

DLLEXPORT void Leviathan::StringIterator::ReInit(const string &text){
	ReInit(new StringClassDataIterator<string>(text), true);
}

DLLEXPORT void Leviathan::StringIterator::ReInit(const wstring* text){
	ReInit(new StringClassPointerIterator<wstring>(text), true);
}

DLLEXPORT void Leviathan::StringIterator::ReInit(const string* text){
	ReInit(new StringClassPointerIterator<string>(text), true);
}
// ------------------------------------ //
DLLEXPORT int Leviathan::StringIterator::GetPreviousCharacter() {
	int tmpval = -1;
	if(!DataIterator->GetPreviousCharacter(tmpval)){
		// Darn //
        ITR_COREDEBUG("Failed to get previous character");
		return 0;
	}

    ITR_COREDEBUG("Peek back char: (" + Convert::CodePointToUtf8(tmpval) + ")");
	return tmpval;
}
// ------------------------------------ //
DLLEXPORT void Leviathan::StringIterator::SkipLineEnd() {

    ITR_COREDEBUG("Skip line end");

    const auto current = GetCharacter(0);
    const auto next = GetCharacter(1);

    // Move past the current new line //
    MoveToNext();

    // Skip multi part new lines //
    if (StringOperations::IsLineTerminator(current, next))
        MoveToNext();

}

