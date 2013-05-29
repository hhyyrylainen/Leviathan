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
	if(data->Positions[1] == -1){
		// set to end on string end //
		data->Positions.Val[1] = GetWstringLenght()-1;
	}

	if(IsPtrUsed){

		resultstr = unique_ptr<wstring>(new wstring(Data->substr(data->Positions[0], data->Positions[1]-data->Positions[0]+1)));
	} else {

		resultstr = unique_ptr<wstring>(new wstring(ConstData.substr(data->Positions[0], data->Positions[1]-data->Positions[0]+1)));
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
	if(data.Positions[0] == -1 && data.Positions[1] == -1){
		resultstr = unique_ptr<wstring>(new wstring(L""));
		return resultstr;
	}

	// check for end //
	if(data.Positions[1] == -1){
		// set to end on string end //
		data.Positions.Val[1] = GetWstringLenght()-1;
	}

	if(IsPtrUsed){

		resultstr = unique_ptr<wstring>(new wstring(Data->substr(data.Positions[0], data.Positions[1]-data.Positions[0]+1)));
	} else {

		resultstr = unique_ptr<wstring>(new wstring(ConstData.substr(data.Positions[0], data.Positions[1]-data.Positions[0]+1)));
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

	if(data->Positions[0] == -1){
		goto getnextnumberfuncendreleaseresourceslabel;
	}

	// create substring of the wanted part //


	// check for end //
	if(data->Positions[1] == -1){
		// set to end on string end //
		data->Positions.Val[1] = GetWstringLenght()-1;
	}

	if(IsPtrUsed){

		resultstr = unique_ptr<wstring>(new wstring(Data->substr(data->Positions[0], data->Positions[1]-data->Positions[0]+1)));
	} else {

		resultstr = unique_ptr<wstring>(new wstring(ConstData.substr(data->Positions[0], data->Positions[1]-data->Positions[0]+1)));
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
	if(data.Positions[0] == data.Positions[1] || data.SeparatorFound == false){
		// nothing found //
		resultstr = unique_ptr<wstring>(new wstring());
		return resultstr;
	}
	if(data.Positions[1] == -1){
		// set to start, this only happens if there is just one character //
		data.Positions.Val[1] = data.Positions[0];
	}

	if(IsPtrUsed){

		resultstr = unique_ptr<wstring>(new wstring(Data->substr(data.Positions[0], data.Positions[1]-data.Positions[0]+1)));
	} else {

		resultstr = unique_ptr<wstring>(new wstring(ConstData.substr(data.Positions[0], data.Positions[1]-data.Positions[0]+1)));
	}

	// return wanted part //
	return resultstr;
}

// ------------------------------------ //
DLLEXPORT Object* Leviathan::WstringIterator::StartIterating(IteratorWstrCallBack functiontocall, Object* IteratorData, int parameters){
	// "switch" here based on what type of wstring is mounted //
	int retval = 0;
	if(IsPtrUsed){
		for(IteratorPosition; IteratorPosition < Data->size(); IteratorPosition++){

			retval = HandleCurrentIteration(functiontocall, IteratorData, parameters);

			if(retval == 1){
				break;
			}
		}
	} else {
		for(IteratorPosition; IteratorPosition < ConstData.size(); IteratorPosition++){

			retval = HandleCurrentIteration(functiontocall, IteratorData, parameters);

			if(retval == 1){
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
			return 1;
		}
		break;
	}

	// check current character //
	switch(HandleSpecialCharacters()){
	case ITERATORCALLBACK_RETURNTYPE_STOP:
		{
			// needs to stop //
			return 1;
		}
		break;
	}

	// valid character/valid iteration call callback //
	switch(functiontocall(this, IteratorData, parameters)){
	case ITERATORCALLBACK_RETURNTYPE_STOP:
		{
			// needs to stop //
			return 1;
		}
		break;
	}

	// reset 1 character long flags //
	CurrentFlags->UnsetFlag(Flag(WSTRINGITERATOR_IGNORE_SPECIAL));
	return 0;
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

DLLEXPORT unsigned int Leviathan::WstringIterator::GetWstringLenght(){
	// switch on wstring type
	if(IsPtrUsed){
		if(Data == NULL){
			//WstringIterator: GetWstringLenght:
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
		if(tmpdata->Positions[0] == -1){
			// first position! //

			

			tmpdata->Positions.Val[0] = instance->IteratorPosition+1;

		} else {
			// end found! //

			tmpdata->Positions.Val[1] = instance->IteratorPosition-1;
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
		if(tmpdata->Positions[0] == -1){
			// first position! //

			tmpdata->Positions.Val[0] = instance->IteratorPosition;
		}

	} else {
		// check for end //
		if(tmpdata->Positions[0] != -1){
			// ended //
			tmpdata->Positions.Val[1] = instance->IteratorPosition-1;
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

	if(((charvalue >= 32) && (charvalue <= 57)) || ((charvalue >= 63) && (charvalue <= 90)) || ((charvalue >= 96) && (charvalue <= 122))){
		// is just a ascii char with some text characters included //
		IsValid = true;
	} else {
		if(stoptype != UNNORMALCHARACTER_TYPE_NON_ASCII){
			// we can check if it allows some other characters //
			if(stoptype == UNNORMALCHARACTER_TYPE_CONTROLCHARACTERS){
				if(!((charvalue >= 91) && (charvalue <= 93)) && !((charvalue >= 58) && (charvalue <= 62)) && ((charvalue < 123) && (charvalue >= 32))){
					// is still valid! //
					IsValid = true;
				}
			}
		}
	}


	if(IsValid){
		// check is this first character //
		if(tmpdata->Positions[0] == -1){
			// first position! //

			tmpdata->Positions.Val[0] = instance->IteratorPosition;
		}

	} else {
		// check for end //
		if(tmpdata->Positions[0] != -1){
			// ended //
			tmpdata->Positions.Val[1] = instance->IteratorPosition-1;
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

			IsStop = true;
		}
	} else {
		if(stoptype == EQUALITYCHARACTER_TYPE_EQUALITY){
			// check for equality sign //
			if(charvalue == (int)'='){

				IsStop = true;
			}
		} else if (stoptype == EQUALITYCHARACTER_TYPE_DOUBLEDOTSTYLE){
			// check does it match the characters //
			if(charvalue == (int)':'){

				IsStop = true;
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
		if(tmpdata->Positions[0] == -1){
			// first position! //
			tmpdata->Positions.Val[0] = instance->IteratorPosition;
		} else {
			// set end to this valid character //
			tmpdata->Positions.Val[1] = instance->IteratorPosition;
		}

	}
	// will have exited if encountered separator character //
	return ITERATORCALLBACK_RETURNTYPE_CONTINUE;
}
