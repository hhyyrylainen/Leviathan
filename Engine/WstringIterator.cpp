#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_WSTRINGITERATOR
#include "WstringIterator.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT Leviathan::WstringIterator::WstringIterator(const wstring& text) : ConstData(text), CurrentFlags(new MultiFlag()){
	HandlesDelete = false;
	Data = NULL;

	// start from beginning of string //
	IteratorPosition = 0;

	// set right type //
	IsPtrUsed = false;
}

DLLEXPORT Leviathan::WstringIterator::WstringIterator(wstring* text, bool TakesOwnership /*= false*/) : CurrentFlags(new MultiFlag()){
	// only delete if wanted //
	HandlesDelete = TakesOwnership;
	Data = text;

	// start from beginning of string //
	IteratorPosition = 0;

	// set right type //
	IsPtrUsed = true;
}

DLLEXPORT Leviathan::WstringIterator::~WstringIterator(){
	if(HandlesDelete){

		SAFE_DELETE(Data);
	}
}
// ------------------------------------ //
DLLEXPORT unsigned long Leviathan::WstringIterator::GetPosition(){
	return IteratorPosition;
}

DLLEXPORT void Leviathan::WstringIterator::SetPosition(unsigned long pos){
	if(IsOutOfBounds(pos)){

		DEBUG_BREAK;
	}
	// update position //
	IteratorPosition = pos;
}
// ------------------------------------ //
DLLEXPORT unique_ptr<wstring> Leviathan::WstringIterator::GetStringInQuotes(QUOTETYPE quotes, bool AllowSpecialQualifiers /*= true*/){
	// iterate over the string and return what is wanted //
	IteratorPositionData* data = new IteratorPositionData();
	data->Positions.SetData(-1, -1);

	// iterate over the string getting the proper part //

	StartIterating(FindFirstQuotedString, (Object*)data, (int)quotes);

	// create substring of the wanted part //
	unique_ptr<wstring> resultstr;

	// check for end //
	if(data->Positions.Y == -1){
		// set to end on string end //
		data->Positions.Y = GetWstringLength()-1;
	}

	if(IsPtrUsed){

		resultstr = unique_ptr<wstring>(new wstring(Data->substr(data->Positions.X, data->Positions.Y-data->Positions.X+1)));
	} else {

		resultstr = unique_ptr<wstring>(new wstring(ConstData.substr(data->Positions.X, data->Positions.Y-data->Positions.X+1)));
	}


	// release memory //
	SAFE_DELETE(data);

	// return wanted part //
	return resultstr;
}


DLLEXPORT unique_ptr<wstring> Leviathan::WstringIterator::GetNextCharacterSequence(UNNORMALCHARACTER stopcase){
	// iterate over the string and return what is wanted //
	IteratorPositionData data;
	data.Positions.SetData(-1, -1);

	// iterate over the string getting the proper part //

	StartIterating(FindNextNormalCharacterString, (Object*)&data, (int)stopcase);

	// create substring of the wanted part //
	unique_ptr<wstring> resultstr;

	// check for nothing found //
	if(data.Positions.X == -1 && data.Positions.Y == -1){
		resultstr = unique_ptr<wstring>(new wstring(L""));
		return resultstr;
	}

	// check for end //
	if(data.Positions.Y == -1){
		// set to end on string end //
		data.Positions.Y = GetWstringLength()-1;
	}

	if(IsPtrUsed){

		resultstr = unique_ptr<wstring>(new wstring(Data->substr(data.Positions.X, data.Positions.Y-data.Positions.X+1)));
	} else {

		resultstr = unique_ptr<wstring>(new wstring(ConstData.substr(data.Positions.X, data.Positions.Y-data.Positions.X+1)));
	}

	// return wanted part //
	return resultstr;
}

DLLEXPORT unique_ptr<wstring> Leviathan::WstringIterator::GetNextNumber(DECIMALSEPARATORTYPE decimal){
	// iterate over the string and return what is wanted //
	IteratorNumberFindData* data = new IteratorNumberFindData();

	// iterate over the string getting the proper part //

	StartIterating(FindNextNumber, (Object*)data, (int)decimal);

	unique_ptr<wstring> resultstr;

	if(data->Positions.X == -1){
		goto getnextnumberfuncendreleaseresourceslabel;
	}

	// create substring of the wanted part //


	// check for end //
	if(data->Positions.Y == -1){
		// set to end on string end //
		data->Positions.Y = GetWstringLength()-1;
	}

	if(IsPtrUsed){

		resultstr = unique_ptr<wstring>(new wstring(Data->substr(data->Positions.X, data->Positions.Y-data->Positions.X+1)));
	} else {

		resultstr = unique_ptr<wstring>(new wstring(ConstData.substr(data->Positions.X, data->Positions.Y-data->Positions.X+1)));
	}

getnextnumberfuncendreleaseresourceslabel:

	// release memory //
	SAFE_DELETE(data);

	// return wanted part //
	return resultstr;
}

DLLEXPORT unique_ptr<wstring> Leviathan::WstringIterator::GetUntilEqualityAssignment(EQUALITYCHARACTER stopcase){
	
	// iterate over the string and return what is wanted //
	IteratorAssignmentData data;
	data.Positions.SetData(-1, -1);
	data.SeparatorFound = false;

	// iterate over the string getting the proper part //
	StartIterating(FindUntilEquality, (Object*)&data, (int)stopcase);

	// create substring of the wanted part //
	unique_ptr<wstring> resultstr;

	// check for end //
	if(data.Positions.X == data.Positions.Y || data.SeparatorFound == false){
		// nothing found //
		resultstr = unique_ptr<wstring>(new wstring());
		return resultstr;
	}
	if(data.Positions.Y == -1){
		// set to start, this only happens if there is just one character //
		data.Positions.Y = data.Positions.X;
	}

	if(IsPtrUsed){

		resultstr = unique_ptr<wstring>(new wstring(Data->substr(data.Positions.X, data.Positions.Y-data.Positions.X+1)));
	} else {

		resultstr = unique_ptr<wstring>(new wstring(ConstData.substr(data.Positions.X, data.Positions.Y-data.Positions.X+1)));
	}

	// return wanted part //
	return resultstr;
}
// ------------------------------------ //
DLLEXPORT void Leviathan::WstringIterator::SkipWhiteSpace(){
	// iterate over the string skipping until hit something that doesn't need to be skipped //
	StartIterating(SkipSomething, NULL, (int)UNNORMALCHARACTER_TYPE_LOWCODES_WHITESPACE);
}
// ------------------------------------ //
DLLEXPORT unique_ptr<wstring> Leviathan::WstringIterator::GetUntilEnd(){
	// just return the end of the string //
	if(IsPtrUsed){

		return unique_ptr<wstring>(new wstring(Data->substr(IteratorPosition, Data->size()-IteratorPosition)));
	}

	return unique_ptr<wstring>(new wstring(ConstData.substr(IteratorPosition, ConstData.size()-IteratorPosition)));
}

DLLEXPORT unique_ptr<wstring> Leviathan::WstringIterator::GetUntilNextCharacterOrNothing(wchar_t charactertolookfor){
	// iterate over the string and return what is wanted //
	IteratorPositionData data;
	data.Positions.SetData(-1, -1);

	// iterate over the string getting the proper part //
	StartIterating(FindUntilSpecificCharacter, (Object*)&data, (int)charactertolookfor);

	// create substring of the wanted part //
	unique_ptr<wstring> resultstr;

	// check for end //
	if(data.Positions.Y == -1 || data.Positions.X == data.Positions.Y){
		// not found the ending character or start character was it //
		return unique_ptr<wstring>(new wstring(L""));
	}

	// return wanted part //
	if(IsPtrUsed){

		return unique_ptr<wstring>(new wstring(Data->substr(data.Positions.X, data.Positions.Y-data.Positions.X+1)));
	} else {

		return unique_ptr<wstring>(new wstring(ConstData.substr(data.Positions.X, data.Positions.Y-data.Positions.X+1)));
	}
}

// ------------------------------------ //
DLLEXPORT Object* Leviathan::WstringIterator::StartIterating(IteratorWstrCallBack functiontocall, Object* IteratorData, int parameters){
	// "switch" here based on what type of wstring is mounted //
	int retval = 0;
	if(IsPtrUsed){
		for(IteratorPosition; IteratorPosition < Data->size(); IteratorPosition++){

			retval = HandleCurrentIteration(functiontocall, IteratorData, parameters);

			if(retval == ITERATORCALLBACK_RETURNTYPE_STOP){
				// try moving to next character //
				//IteratorPosition++;
				break;
			}
		}
	} else {
		for(IteratorPosition; IteratorPosition < ConstData.size(); IteratorPosition++){

			retval = HandleCurrentIteration(functiontocall, IteratorData, parameters);

			if(retval == ITERATORCALLBACK_RETURNTYPE_STOP){
				// try moving to next character //
				//IteratorPosition++;
				break;
			}
		}
	}

	return IteratorData;
}

Leviathan::ITERATORCALLBACK_RETURNTYPE Leviathan::WstringIterator::HandleSpecialCharacters(){
	// check should this special character be ignored //
	if(CurrentFlags->IsSet(WSTRINGITERATOR_IGNORE_SPECIAL))
		return ITERATORCALLBACK_RETURNTYPE_CONTINUE;


	// check for special characters //
	wchar_t character = GetCurrentCharacter();

	switch(character){
	case L'\\':
		{
			// ignore next special character //
			CurrentFlags->SetFlag(Flag(WSTRINGITERATOR_IGNORE_SPECIAL));
		}
	break;
	case L'"':
		{
			// a string //
			if(!CurrentFlags->IsSet(WSTRINGITERATOR_INSIDE_STRING_DOUBLE)){
				// set //
				CurrentFlags->SetFlag(Flag(WSTRINGITERATOR_INSIDE_STRING_DOUBLE));

				// set as inside string //
				CurrentFlags->SetFlag(Flag(WSTRINGITERATOR_INSIDE_STRING));

			} else {
				// unset flag //
				CurrentFlags->UnsetFlag(Flag(WSTRINGITERATOR_INSIDE_STRING_DOUBLE));

				// check can we unset whole string flag //
				if(!CurrentFlags->IsSet(WSTRINGITERATOR_INSIDE_STRING_SINGLE)){
					// can unset this //
					CurrentFlags->UnsetFlag(Flag(WSTRINGITERATOR_INSIDE_STRING));
				}
			}
		}
	break;
	case L'\'':
		{
			// a string //
			if(!CurrentFlags->IsSet(WSTRINGITERATOR_INSIDE_STRING_SINGLE)){
				// set //
				CurrentFlags->SetFlag(Flag(WSTRINGITERATOR_INSIDE_STRING_SINGLE));

				// set as inside string //
				CurrentFlags->SetFlag(Flag(WSTRINGITERATOR_INSIDE_STRING));

			} else {
				// unset flag //
				CurrentFlags->UnsetFlag(Flag(WSTRINGITERATOR_INSIDE_STRING_SINGLE));

				// check can we unset whole string flag //
				if(!CurrentFlags->IsSet(WSTRINGITERATOR_INSIDE_STRING_DOUBLE)){
					// can unset this //
					CurrentFlags->UnsetFlag(Flag(WSTRINGITERATOR_INSIDE_STRING));
				}
			}
		}
		break;

	}

	return ITERATORCALLBACK_RETURNTYPE_CONTINUE;
}

Leviathan::ITERATORCALLBACK_RETURNTYPE Leviathan::WstringIterator::CheckActiveFlags(){
	if(CurrentFlags->IsSet(WSTRINGITERATOR_STOP))
		return ITERATORCALLBACK_RETURNTYPE_STOP;

	return ITERATORCALLBACK_RETURNTYPE_CONTINUE;
}

int Leviathan::WstringIterator::HandleCurrentIteration(IteratorWstrCallBack functiontocall, Object* IteratorData, int parameters){
	switch(CheckActiveFlags()){
	case ITERATORCALLBACK_RETURNTYPE_STOP:
		{
			// needs to stop //
			return ITERATORCALLBACK_RETURNTYPE_STOP;
		}
		break;
	}

	// check current character //
	switch(HandleSpecialCharacters()){
	case ITERATORCALLBACK_RETURNTYPE_STOP:
		{
			// needs to stop //
			return ITERATORCALLBACK_RETURNTYPE_STOP;
		}
		break;
	}

	// valid character/valid iteration call callback //
	switch(functiontocall(this, IteratorData, parameters)){
	case ITERATORCALLBACK_RETURNTYPE_STOP:
		{
			// needs to stop //
			return ITERATORCALLBACK_RETURNTYPE_STOP;
		}
		break;
	}

	// reset 1 character long flags //
	if(CurrentFlags->IsSet(WSTRINGITERATOR_IGNORE_SPECIAL)){
		// check should end now //
		if(CurrentFlags->IsSet(WSTRINGITERATOR_IGNORE_SPECIAL_END)){
			// unset both //
			CurrentFlags->UnsetFlag(Flag(WSTRINGITERATOR_IGNORE_SPECIAL_END));
			CurrentFlags->UnsetFlag(Flag(WSTRINGITERATOR_IGNORE_SPECIAL));
		} else {
			// set to end next character //
			CurrentFlags->SetFlag(Flag(WSTRINGITERATOR_IGNORE_SPECIAL_END));
		}
	}
	return ITERATORCALLBACK_RETURNTYPE_CONTINUE;
}

DLLEXPORT bool Leviathan::WstringIterator::IsOutOfBounds(unsigned long pos){
	// switch on wstring type
	if(IsPtrUsed){
		if(Data == NULL){
			throw ExceptionNULLPtr(L"Text pointer is invalid (while checking pos)", pos, __WFUNCTION__, (void*)Data);
		}
		if(pos >= Data->size()){
			return true;
		}
	} else {
		if(pos >= ConstData.size()){
			return true;
		}
	}
	return false;
}

DLLEXPORT bool Leviathan::WstringIterator::IsOutOfBounds(){
	if(IsPtrUsed){
		if(Data == NULL){
			throw ExceptionNULLPtr(L"Text pointer is invalid (while checking IsOutOfBounds)", 0, __WFUNCTION__, (void*)Data);
		}
		if(IteratorPosition >= Data->size()){
			return true;
		}
	} else {
		if(IteratorPosition >= ConstData.size()){
			return true;
		}
	}
	return false;
}

DLLEXPORT unsigned int Leviathan::WstringIterator::GetWstringLength(){
	// switch on wstring type
	if(IsPtrUsed){
		if(Data == NULL){
			//WstringIterator: GetWstringLength:
			throw ExceptionNULLPtr(L"Text pointer is invalid ", NULL, __WFUNCTION__, (void*)Data);
		}
		return Data->size();

	} else {
		return ConstData.size();

	}
}

DLLEXPORT wchar_t Leviathan::WstringIterator::GetCurrentCharacter(){
	if(IsPtrUsed){

		return (*this->Data)[this->IteratorPosition];
	} else {

		return this->ConstData[this->IteratorPosition];
	}
}

DLLEXPORT wchar_t Leviathan::WstringIterator::GetCharacterAtPos(size_t pos){
	// TODO: put a index check here //
	if(IsPtrUsed){

		return (*this->Data)[pos];
	} else {

		return this->ConstData[pos];
	}
}

DLLEXPORT bool Leviathan::WstringIterator::MoveToNext(){
	IteratorPosition++;
	// return true if it is still valid //
	if(IsPtrUsed){

		return Data->size() > IteratorPosition;
	} else {

		return ConstData.size() > IteratorPosition;
	}
}

DLLEXPORT void Leviathan::WstringIterator::ReInit(wstring* text, bool TakesOwnership /*= false*/){
	// copied from ctor //

	// only delete if wanted //
	HandlesDelete = TakesOwnership;
	Data = text;

	// start from beginning of string //
	IteratorPosition = 0;

	// set right type //
	IsPtrUsed = true;

	// clear flags //
	CurrentFlags->ClearFlags();
}

DLLEXPORT void Leviathan::WstringIterator::ReInit(const wstring& text){
	// copied from ctor //
	HandlesDelete = false;
	Data = NULL;

	// start from beginning of string //
	IteratorPosition = 0;

	// set right type //
	IsPtrUsed = false;

	// clear flags //
	CurrentFlags->ClearFlags();
}

DLLEXPORT void Leviathan::WstringIterator::StripPreceedingAndTrailingWhitespaceComments(wstring &str){
	// create iterator for finding the right parts //
	WstringIterator itr(&str, false);

	// iterate over the string and return what is wanted //
	IteratorPositionData data(-1, -1);

	// iterate over the string getting the proper part //
	itr.StartIterating(FindFromStartUntilCommentOrEnd, (Object*)&data, (int)0);

	// check for nothing/1 character found //
	if(data.Positions.X == data.Positions.Y){
		if(data.Positions.X == -1){
			// nothing found //
			str.clear();
			return;
		} else {
			// just one character //
			str = str[data.Positions.X];
			return;
		}
	}

	// set the string as the only wanted part
	str = str.substr(data.Positions.X, data.Positions.Y-data.Positions.X+1);
}

// ------------------------------------ //
ITERATORCALLBACK_RETURNTYPE Leviathan::FindFirstQuotedString(WstringIterator* instance, Object* IteratorData, int parameters){
	// check is current element a quote //
	wchar_t CurChar(instance->GetCurrentCharacter());

	bool IsQuote = false;

	// check for quote //
	QUOTETYPE quotetype = (QUOTETYPE)parameters;

	switch(quotetype){
	case QUOTETYPE_BOTH:
		{
			if(CurChar == L'"')
				IsQuote = true;
			if(CurChar == L'\'')
				IsQuote = true;
		}
	break;
	case QUOTETYPE_SINGLEQUOTES:
		{
			if(CurChar == L'\'')
				IsQuote = true;
		}
	break;
	case QUOTETYPE_DOUBLEQUOTES:
		{
			if(CurChar == L'"')
				IsQuote = true;
		}
	break;
	}

	if(IsQuote){
		// check for skipping //
		if(instance->CurrentFlags->IsSet(WSTRINGITERATOR_IGNORE_SPECIAL)){

			return ITERATORCALLBACK_RETURNTYPE_CONTINUE;
		}


		IteratorPositionData* tmpdata = dynamic_cast<IteratorPositionData*>(IteratorData);
		if(tmpdata == NULL){
			// well darn //
			DEBUG_BREAK;
		}

		// check is this first quote //
		if(tmpdata->Positions.X == -1){
			// first position! //

			

			tmpdata->Positions.X = instance->IteratorPosition+1;

		} else {
			// end found! //

			tmpdata->Positions.Y = instance->IteratorPosition-1;
			return ITERATORCALLBACK_RETURNTYPE_STOP;
		}

	}
	return ITERATORCALLBACK_RETURNTYPE_CONTINUE;
}

Leviathan::ITERATORCALLBACK_RETURNTYPE Leviathan::FindNextNumber(WstringIterator* instance, Object* IteratorData, int parameters){
	// check is current element a part of number //
	wchar_t CurChar(instance->GetCurrentCharacter());

	bool IsValid = false;

	// check for number //
	DECIMALSEPARATORTYPE decimaltype = (DECIMALSEPARATORTYPE)parameters;

	IteratorNumberFindData* tmpdata = dynamic_cast<IteratorNumberFindData*>(IteratorData);
	if(tmpdata == NULL){
		// well darn //
		DEBUG_BREAK;
	}

	if((((int)CurChar) >= 48) && (((int)CurChar) <= 57)){
		// is a plain old digit //
		IsValid = true;
	} else {
		// check is it decimal separator (1 allowed) or negativity sign in from //
		if(CurChar == L'+' || CurChar == L'-'){

			if((tmpdata->DigitsFound < 1) && (!tmpdata->NegativeFound)){
				IsValid = true;
			}
			tmpdata->NegativeFound = true;
		} else if (((CurChar == L'.') && ((decimaltype == DECIMALSEPARATORTYPE_DOT) || (decimaltype == DECIMALSEPARATORTYPE_BOTH))) ||
			((CurChar == L',') && ((decimaltype == DECIMALSEPARATORTYPE_COMMA) || (decimaltype == DECIMALSEPARATORTYPE_BOTH))))
		{
			if((!tmpdata->DecimalFound) && (tmpdata->DigitsFound > 0)){
				IsValid = true;
				tmpdata->DecimalFound = true;
			}
			
		}
	}



	if(IsValid){
		// check is this first digit //
		tmpdata->DigitsFound++;
		if(tmpdata->Positions.X == -1){
			// first position! //

			tmpdata->Positions.X = instance->IteratorPosition;
		}

	} else {
		// check for end //
		if(tmpdata->Positions.X != -1){
			// ended //
			tmpdata->Positions.Y = instance->IteratorPosition-1;
			return ITERATORCALLBACK_RETURNTYPE_STOP;
		}

	}
	return ITERATORCALLBACK_RETURNTYPE_CONTINUE;
}

ITERATORCALLBACK_RETURNTYPE Leviathan::FindNextNormalCharacterString(WstringIterator* instance, Object* IteratorData, int parameters){
	// check is current element a valid element //
	wchar_t CurChar(instance->GetCurrentCharacter());

	bool IsValid = false;

	// check for number //
	UNNORMALCHARACTER stoptype = (UNNORMALCHARACTER)parameters;

	IteratorPositionData* tmpdata = dynamic_cast<IteratorPositionData*>(IteratorData);
	if(tmpdata == NULL){
		// well darn //
		DEBUG_BREAK;
		return ITERATORCALLBACK_RETURNTYPE_STOP;
	}
	int charvalue = (int) CurChar;

	if(stoptype == UNNORMALCHARACTER_TYPE_LOWCODES_WHITESPACE){
		// check for whitespace //
		if(charvalue < 33){
			IsValid = false;
		} else {
			IsValid = true;
		}
	} else {
		if(((charvalue >= 32) && (charvalue <= 57)) || ((charvalue >= 63) && (charvalue <= 90)) || ((charvalue >= 96) && (charvalue <= 122))){
			// is just a ascii char with some text characters included //

			if(stoptype == UNNORMALCHARACTER_TYPE_NON_NAMEVALID_WITHWHITESPACE){

				if((charvalue >= 48 && charvalue <= 57) || (charvalue >= 64 && charvalue <= 90) || (charvalue >= 97 && charvalue <= 122)){
					IsValid = true;
				}

			} else {
				IsValid = true;
			}
			
		} else {
			if((stoptype != UNNORMALCHARACTER_TYPE_NON_ASCII)){
				// we can check if it allows some other characters //
				if(stoptype == UNNORMALCHARACTER_TYPE_CONTROLCHARACTERS){
					// skip if this character is to be ignored //
					if(instance->CurrentFlags->IsSet(WSTRINGITERATOR_IGNORE_SPECIAL)){

						IsValid = true;
					} else {
						if(!((charvalue >= 91) && (charvalue <= 93)) && !((charvalue >= 58) && (charvalue <= 62)) && ((charvalue < 123) 
							&& (charvalue >= 32)))
						{
							// is still valid! //
							IsValid = true;
						}
					}
				} else if (stoptype == UNNORMALCHARACTER_TYPE_NON_NAMEVALID_WITHWHITESPACE){
					// check for whitespace //
					if(charvalue >= 2 && charvalue <= 32){

						IsValid = true;
					}
				}
			}
		}
	}


	if(IsValid){
		// check is this first character //
		if(tmpdata->Positions.X == -1){
			// first position! //

			tmpdata->Positions.X = instance->IteratorPosition;
		}

	} else {
		// check for end //
		if(tmpdata->Positions.X != -1){
			// ended //
			tmpdata->Positions.Y = instance->IteratorPosition-1;
			return ITERATORCALLBACK_RETURNTYPE_STOP;
		}
	}

	return ITERATORCALLBACK_RETURNTYPE_CONTINUE;
}

Leviathan::ITERATORCALLBACK_RETURNTYPE Leviathan::FindUntilEquality(WstringIterator* instance, Object* IteratorData, int parameters){
	// check is current element a valid element //
	int charvalue((int)instance->GetCurrentCharacter());

	bool IsValid = true;
	bool IsStop = false;

	// what characters are stopping //
	EQUALITYCHARACTER stoptype = (EQUALITYCHARACTER)parameters;

	IteratorAssignmentData* tmpdata = dynamic_cast<IteratorAssignmentData*>(IteratorData);
	if(tmpdata == NULL){
		// well darn //
		DEBUG_BREAK;
		return ITERATORCALLBACK_RETURNTYPE_STOP;
	}

	// skip if this is a space //
	if((charvalue < 33)){
		// non allowed character in name
		IsValid = false;
	}

	if(stoptype == EQUALITYCHARACTER_TYPE_ALL){
		// check for all possible value separators //
		if(charvalue == (int)'=' || charvalue == (int)':'){

			if(!instance->CurrentFlags->IsSet(WSTRINGITERATOR_IGNORE_SPECIAL)){
				// if ignored don't stop //
				IsStop = true;
			}
		}
	} else {
		if(stoptype == EQUALITYCHARACTER_TYPE_EQUALITY){
			// check for equality sign //
			if(charvalue == (int)'='){
				if(!instance->CurrentFlags->IsSet(WSTRINGITERATOR_IGNORE_SPECIAL)){
					IsStop = true;
				}
			}
		} else if (stoptype == EQUALITYCHARACTER_TYPE_DOUBLEDOTSTYLE){
			// check does it match the characters //
			if(charvalue == (int)':'){
				if(!instance->CurrentFlags->IsSet(WSTRINGITERATOR_IGNORE_SPECIAL)){
					IsStop = true;
				}
			}
		}
	}

	if(!IsStop){
		// end if end already found //
		if(tmpdata->SeparatorFound){
			return ITERATORCALLBACK_RETURNTYPE_STOP;
		}
	} else {
		tmpdata->SeparatorFound = true;
		IsValid = false;
	}

	if(IsValid){
		// check is this first character //
		if(tmpdata->Positions.X == -1){
			// first position! //
			tmpdata->Positions.X = instance->IteratorPosition;
		} else {
			// set end to this valid character //
			tmpdata->Positions.Y = instance->IteratorPosition;
		}

	}
	// will have exited if encountered separator character //
	return ITERATORCALLBACK_RETURNTYPE_CONTINUE;
}

Leviathan::ITERATORCALLBACK_RETURNTYPE Leviathan::FindFromStartUntilCommentOrEnd(WstringIterator* instance, Object* IteratorData, int parameters){
	// we can just return if we are inside a string //
	if(instance->CurrentFlags->IsSet(WSTRINGITERATOR_INSIDE_STRING)){
		// position is always valid inside string, goto end for this being valid //
		goto findfromstartuntilcommentorendfuncendlabel;
		//return ITERATORCALLBACK_RETURNTYPE_CONTINUE;
	}

	int charvalue((int)instance->GetCurrentCharacter());
	// check current character //
	if(charvalue < 33){
		// here's nothing to do //
		return ITERATORCALLBACK_RETURNTYPE_CONTINUE;
	}
	// check for some special cases //
	if(charvalue == L'/'){
		// check is next comment //
		if(!instance->IsOutOfBounds(instance->IteratorPosition+1)){
			// check for special ignore //
			if(!instance->CurrentFlags->IsSet(WSTRINGITERATOR_IGNORE_SPECIAL)){
				// check it //
				if(instance->GetCharacterAtPos(instance->IteratorPosition+1) == L'/'){
					// comment started, done //
					return ITERATORCALLBACK_RETURNTYPE_STOP;
				}
			}
		}
	}

findfromstartuntilcommentorendfuncendlabel:
	// get position data //
	IteratorPositionData* tmpdata = dynamic_cast<IteratorPositionData*>(IteratorData);
	if(tmpdata == NULL){
		// well darn //
		DEBUG_BREAK;
		return ITERATORCALLBACK_RETURNTYPE_STOP;
	}

	// set this as last index if not first //
	if(tmpdata->Positions.X == -1){

		// first position //
		tmpdata->Positions.X = instance->IteratorPosition;
		tmpdata->Positions.Y = instance->IteratorPosition;
	} else {
		// currently last position //
		tmpdata->Positions.Y = instance->IteratorPosition;
	}

	return ITERATORCALLBACK_RETURNTYPE_CONTINUE;
}

Leviathan::ITERATORCALLBACK_RETURNTYPE Leviathan::FindUntilSpecificCharacter(WstringIterator* instance, Object* IteratorData, int parameters){
	
	// get position data //
	IteratorPositionData* tmpdata = dynamic_cast<IteratorPositionData*>(IteratorData);
	if(tmpdata == NULL){
		// well darn //
		DEBUG_BREAK;
		return ITERATORCALLBACK_RETURNTYPE_STOP;
	}
	// set start pos if not on invalid character //


	// we can just return if we are inside a string //
	if(instance->CurrentFlags->IsSet(WSTRINGITERATOR_INSIDE_STRING)){
		// valid character set start if not already set //
		if(tmpdata->Positions.X == -1){
			tmpdata->Positions.X = instance->IteratorPosition;
		}

		// can't find specific character inside a string //
		return ITERATORCALLBACK_RETURNTYPE_CONTINUE;
	}
	
	// check for found character //
	if(instance->GetCharacterAtPos(instance->IteratorPosition) == (wchar_t)parameters){
		// skip if ignoring special characters //
		if(instance->CurrentFlags->IsSet(WSTRINGITERATOR_IGNORE_SPECIAL)){
			// valid character set start if not already set //
			if(tmpdata->Positions.X == -1){
				tmpdata->Positions.X = instance->IteratorPosition;
			}
			return ITERATORCALLBACK_RETURNTYPE_CONTINUE;
		}

		if(tmpdata->Positions.X == -1){
			// we haven't started yet, skip //
			return ITERATORCALLBACK_RETURNTYPE_CONTINUE;
		}

		// found character, set last char as the one before this position //
		tmpdata->Positions.Y = instance->IteratorPosition-1;
		return ITERATORCALLBACK_RETURNTYPE_STOP;
	}
	// valid character set start if not already set //
	if(tmpdata->Positions.X == -1){
		tmpdata->Positions.X = instance->IteratorPosition;
	}
	// just continue //
	return ITERATORCALLBACK_RETURNTYPE_CONTINUE;
}

Leviathan::ITERATORCALLBACK_RETURNTYPE Leviathan::SkipSomething(WstringIterator* instance, Object* notwanted, int parameters){

	// we can just return if we are inside a string //
	if(instance->CurrentFlags->IsSet(WSTRINGITERATOR_INSIDE_STRING)){
		return ITERATORCALLBACK_RETURNTYPE_CONTINUE;
	}
	int curchara = (int)instance->GetCurrentCharacter();
	// check does character match what is skipped //

	UNNORMALCHARACTER stoptype = (UNNORMALCHARACTER)parameters;

	switch(stoptype){
	case UNNORMALCHARACTER_TYPE_LOWCODES_WHITESPACE:
		{
			if(curchara <= 32)
				return ITERATORCALLBACK_RETURNTYPE_CONTINUE;
		}
		break;

	default:
		return ITERATORCALLBACK_RETURNTYPE_STOP;
	}
	// didn't match to be skipped characters //
	return ITERATORCALLBACK_RETURNTYPE_STOP;
}
