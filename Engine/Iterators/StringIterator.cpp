#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_STRINGITERATOR
#include "StringIterator.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT Leviathan::StringIterator::StringIterator(StringDataIterator* iterator, bool TakesOwnership = false) : CurrentFlags(0), 
	HandlesDelete(TakesOwnership), DataIterator(iterator)
{
#ifdef _DEBUG
	DebugMode = false;
#endif // _DEBUG
}

DLLEXPORT Leviathan::StringIterator::StringIterator(const string &text) : CurrentFlags(0), HandlesDelete(true), 
	DataIterator(new StringClassDataIterator<string>(text))
{
#ifdef _DEBUG
	DebugMode = false;
#endif // _DEBUG
}

DLLEXPORT Leviathan::StringIterator::StringIterator(const wstring &text) : CurrentFlags(0), HandlesDelete(true), 
	DataIterator(new StringClassDataIterator<wstring>(text))
{
#ifdef _DEBUG
	DebugMode = false;
#endif // _DEBUG
}

DLLEXPORT Leviathan::StringIterator::~StringIterator(){
	if(HandlesDelete){

		SAFE_DELETE(DataIterator);
	}
}
// ------------------------------------ //
DLLEXPORT unique_ptr<wstring> Leviathan::StringIterator::GetStringInQuotes(QUOTETYPE quotes){
	// iterate over the string and return what is wanted //
	IteratorPositionData* data = new IteratorPositionData();
	data->Positions.SetData(-1, -1);

	// iterate over the string getting the proper part //

	StartIterating(FindFirstQuotedString, (Object*)data, (int)quotes);

	// create substring of the wanted part //
	unique_ptr<wstring> resultstr;

	// check for nothing found //
	if(data->Positions.X == -1){

		return unique_ptr<wstring>(new wstring(L""));

	}

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


DLLEXPORT unique_ptr<wstring> Leviathan::StringIterator::GetNextCharacterSequence(int stopcaseflags){
	// iterate over the string and return what is wanted //
	IteratorPositionData data;
	data.Positions.SetData(-1, -1);

	// iterate over the string getting the proper part //

	StartIterating(FindNextNormalCharacterString, (Object*)&data, (int)stopcaseflags);

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

DLLEXPORT unique_ptr<wstring> Leviathan::StringIterator::GetNextNumber(DECIMALSEPARATORTYPE decimal){
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

DLLEXPORT unique_ptr<wstring> Leviathan::StringIterator::GetUntilEqualityAssignment(EQUALITYCHARACTER stopcase){

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
DLLEXPORT void Leviathan::StringIterator::SkipWhiteSpace(){
	// iterate over the string skipping until hit something that doesn't need to be skipped //
	StartIterating(SkipSomething, NULL, (int)UNNORMALCHARACTER_TYPE_LOWCODES);
}

DLLEXPORT void Leviathan::StringIterator::SkipCharacters(wchar_t chartoskip){
	IteratorCharacterData stufftoskip(chartoskip);

	// iterate over the string skipping until hit something that doesn't need to be skipped //
	StartIterating(SkipSomething, &stufftoskip, 0);
}
// ------------------------------------ //
DLLEXPORT unique_ptr<wstring> Leviathan::StringIterator::GetUntilEnd(){
	// just return the end of the string //
	if(IsPtrUsed){

		return unique_ptr<wstring>(new wstring(Data->substr(IteratorPosition, Data->size()-IteratorPosition)));
	}

	return unique_ptr<wstring>(new wstring(ConstData.substr(IteratorPosition, ConstData.size()-IteratorPosition)));
}

DLLEXPORT unique_ptr<wstring> Leviathan::StringIterator::GetUntilNextCharacterOrNothing(wchar_t charactertolookfor){
	// iterate over the string and return what is wanted //
	IteratorFindUntilData data;
	data.Positions.SetData(-1, -1);

	// iterate over the string getting the proper part //
	StartIterating(FindUntilSpecificCharacter, (Object*)&data, (int)charactertolookfor);

#ifdef _DEBUG
	if(DebugMode){
		Logger::Get()->Write(L"Iterator: find GetUntilNextCharacterOrAll, positions: "+Convert::ToWstring(data.Positions.X)+L":"
			+Convert::ToWstring(data.Positions.Y)+L", found: "+Convert::ToWstring(data.FoundEnd));
	}
#endif // _DEBUG

	// create substring of the wanted part //
	unique_ptr<wstring> resultstr;

	// check for end //
	if(!data.FoundEnd || data.Positions.X == -1){
		// not found the ending character //
		return unique_ptr<wstring>(new wstring(L""));
	}

	// return wanted part //
	if(IsPtrUsed){

		return unique_ptr<wstring>(new wstring(Data->substr(data.Positions.X, data.Positions.Y-data.Positions.X+1)));
	} else {

		return unique_ptr<wstring>(new wstring(ConstData.substr(data.Positions.X, data.Positions.Y-data.Positions.X+1)));
	}
}

DLLEXPORT unique_ptr<wstring> Leviathan::StringIterator::GetUntilNextCharacterOrAll(wchar_t charactertolookfor){
	// iterate over the string and return what is wanted //
	IteratorFindUntilData data;
	data.Positions.SetData(-1, -1);

	// iterate over the string getting the proper part //
	StartIterating(FindUntilSpecificCharacter, (Object*)&data, (int)charactertolookfor);

#ifdef _DEBUG
	if(DebugMode){
		Logger::Get()->Write(L"Iterator: find GetUntilNextCharacterOrAll, positions: "+Convert::ToWstring(data.Positions.X)+L":"
			+Convert::ToWstring(data.Positions.Y)+L", found: "+Convert::ToWstring(data.FoundEnd));
	}
#endif // _DEBUG

	if(data.Positions.X == -1 || data.Positions.Y == -1){
		// return empty string //
		return unique_ptr<wstring>(new wstring());
	}

	// return wanted part //
	if(IsPtrUsed){

		return unique_ptr<wstring>(new wstring(Data->substr(data.Positions.X, data.Positions.Y-data.Positions.X+1)));
	} else {

		return unique_ptr<wstring>(new wstring(ConstData.substr(data.Positions.X, data.Positions.Y-data.Positions.X+1)));
	}
}

// ------------------------------------ //
DLLEXPORT Object* Leviathan::StringIterator::StartIterating(IteratorWstrCallBack functiontocall, Object* IteratorData, int parameters){
	// loop over string using handle iteration function //

	// we want to skip multiple checks on same character so we skip checks on first character when starting except the beginning of the string //
	bool IsStartUpLoop = IteratorPosition > 0 ? true: false;

	for(IteratorPosition; IteratorPosition < (IsPtrUsed ? Data->size(): ConstData.size()); IteratorPosition++){
#ifdef _DEBUG
		if(DebugMode){
			Logger::Get()->Write(L"Iterator: iterating: "+Convert::ToWstring(IteratorPosition)+L" ("+GetCurrentCharacter()+L")");
		}
#endif // _DEBUG
		auto retval = HandleCurrentIteration(functiontocall, IteratorData, parameters, IsStartUpLoop);

		if(retval == ITERATORCALLBACK_RETURNTYPE_STOP){
			break;
		}
	}


	return IteratorData;
}

Leviathan::ITERATORCALLBACK_RETURNTYPE Leviathan::StringIterator::HandleSpecialCharacters(){
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

#ifdef _DEBUG
			if(DebugMode){
				Logger::Get()->Write(L"Iterator: setting: WSTRINGITERATOR_IGNORE_SPECIAL");
			}
#endif // _DEBUG
		}
	break;
	case L'"':
		{
			// a string //
			if(!CurrentFlags->IsSet(WSTRINGITERATOR_INSIDE_STRING_DOUBLE)){
#ifdef _DEBUG
				if(DebugMode){
					Logger::Get()->Write(L"Iterator: setting: WSTRINGITERATOR_INSIDE_STRING_DOUBLE");
				}
#endif // _DEBUG
				// set //
				CurrentFlags->SetFlag(Flag(WSTRINGITERATOR_INSIDE_STRING_DOUBLE));

				// set as inside string //
				CurrentFlags->SetFlag(Flag(WSTRINGITERATOR_INSIDE_STRING));

			} else {
#ifdef _DEBUG
				if(DebugMode){
					Logger::Get()->Write(L"Iterator: set flag end: WSTRINGITERATOR_INSIDE_STRING_DOUBLE");
				}
#endif // _DEBUG
				// set ending flag //
				CurrentFlags->SetFlag(Flag(WSTRINGITERATOR_INSIDE_STRING_DOUBLE_END));
			}
		}
	break;
	case L'\'':
		{
			// a string //
			if(!CurrentFlags->IsSet(WSTRINGITERATOR_INSIDE_STRING_SINGLE)){
#ifdef _DEBUG
				if(DebugMode){
					Logger::Get()->Write(L"Iterator: setting: WSTRINGITERATOR_INSIDE_STRING_SINGLE");
				}
#endif // _DEBUG
				// set //
				CurrentFlags->SetFlag(Flag(WSTRINGITERATOR_INSIDE_STRING_SINGLE));

				// set as inside string //
				CurrentFlags->SetFlag(Flag(WSTRINGITERATOR_INSIDE_STRING));

			} else {
#ifdef _DEBUG
				if(DebugMode){
					Logger::Get()->Write(L"Iterator: set flag end: WSTRINGITERATOR_INSIDE_STRING_SINGLE");
				}
#endif // _DEBUG
				CurrentFlags->SetFlag(Flag(WSTRINGITERATOR_INSIDE_STRING_SINGLE_END));
			}
		}
		break;

	}

	return ITERATORCALLBACK_RETURNTYPE_CONTINUE;
}

Leviathan::ITERATORCALLBACK_RETURNTYPE Leviathan::StringIterator::CheckActiveFlags(){
	if(CurrentFlags->IsSet(WSTRINGITERATOR_STOP))
		return ITERATORCALLBACK_RETURNTYPE_STOP;

	// reset 1 character long flags //
	if(CurrentFlags->IsSet(WSTRINGITERATOR_IGNORE_SPECIAL)){
#ifdef _DEBUG
		if(DebugMode){
			Logger::Get()->Write(L"Iterator: flag: WSTRINGITERATOR_IGNORE_SPECIAL");
		}
#endif // _DEBUG

		// check should end now //
		if(CurrentFlags->IsSet(WSTRINGITERATOR_IGNORE_SPECIAL_END)){
#ifdef _DEBUG
			if(DebugMode){
				Logger::Get()->Write(L"Iterator: flag ended: WSTRINGITERATOR_IGNORE_SPECIAL");
			}
#endif // _DEBUG
			// unset both //
			CurrentFlags->UnsetFlag(Flag(WSTRINGITERATOR_IGNORE_SPECIAL_END));
			CurrentFlags->UnsetFlag(Flag(WSTRINGITERATOR_IGNORE_SPECIAL));
		} else {
#ifdef _DEBUG
			if(DebugMode){
				Logger::Get()->Write(L"Iterator: flag ends next character: WSTRINGITERATOR_IGNORE_SPECIAL");
			}
#endif // _DEBUG
			// set to end next character //
			CurrentFlags->SetFlag(Flag(WSTRINGITERATOR_IGNORE_SPECIAL_END));
		}
	}

	// reset end flags before we process this cycle further //
	if(CurrentFlags->IsSet(WSTRINGITERATOR_INSIDE_STRING_DOUBLE_END)){
#ifdef _DEBUG
		if(DebugMode){
			Logger::Get()->Write(L"Iterator: flag ends: WSTRINGITERATOR_INSIDE_STRING_DOUBLE");
		}
#endif // _DEBUG
		// unset flag //
		CurrentFlags->UnsetFlag(Flag(WSTRINGITERATOR_INSIDE_STRING_DOUBLE));
		CurrentFlags->UnsetFlag(Flag(WSTRINGITERATOR_INSIDE_STRING_DOUBLE_END));
	}
	if(CurrentFlags->IsSet(WSTRINGITERATOR_INSIDE_STRING_SINGLE_END)){
#ifdef _DEBUG
		if(DebugMode){
			Logger::Get()->Write(L"Iterator: flag ends: WSTRINGITERATOR_INSIDE_STRING_SINGLE");
		}
#endif // _DEBUG
		// unset flag //
		CurrentFlags->UnsetFlag(Flag(WSTRINGITERATOR_INSIDE_STRING_SINGLE));
		CurrentFlags->UnsetFlag(Flag(WSTRINGITERATOR_INSIDE_STRING_SINGLE_END));
	}
	// check can we unset whole string flag //
	if(CurrentFlags->IsSet(WSTRINGITERATOR_INSIDE_STRING)){
		if(!CurrentFlags->IsSet(WSTRINGITERATOR_INSIDE_STRING_DOUBLE) && !CurrentFlags->IsSet(WSTRINGITERATOR_INSIDE_STRING_SINGLE)){
	#ifdef _DEBUG
			if(DebugMode){
				Logger::Get()->Write(L"Iterator: flag ends: WSTRINGITERATOR_INSIDE_STRING");
			}
	#endif // _DEBUG
			// can unset this //
			CurrentFlags->UnsetFlag(Flag(WSTRINGITERATOR_INSIDE_STRING));
		}
	}


	return ITERATORCALLBACK_RETURNTYPE_CONTINUE;
}

int Leviathan::StringIterator::HandleCurrentIteration(IteratorWstrCallBack functiontocall, Object* IteratorData, int parameters, bool &firstiter){
	// first iteration of call is same as last iterations last call //
	if(!firstiter){
#ifdef _DEBUG
		if(DebugMode){
			Logger::Get()->Write(L"Iterator: handle: CheckActiveFlags, HandleSpecialCharacters");
		}
#endif // _DEBUG
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

	}
	firstiter = false;

#ifdef _DEBUG
	if(DebugMode){
		Logger::Get()->Write(L"Iterator: handle: call check function");
	}
#endif // _DEBUG

	// valid character/valid iteration call callback //
	switch(functiontocall(this, IteratorData, parameters)){
	case ITERATORCALLBACK_RETURNTYPE_STOP:
		{
			// needs to stop //
			return ITERATORCALLBACK_RETURNTYPE_STOP;
		}
		break;
	}

	return ITERATORCALLBACK_RETURNTYPE_CONTINUE;
}

DLLEXPORT bool Leviathan::StringIterator::IsOutOfBounds(unsigned long pos){
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

DLLEXPORT bool Leviathan::StringIterator::IsOutOfBounds(){
	return IsOutOfBounds(IteratorPosition);
}

DLLEXPORT unsigned int Leviathan::StringIterator::GetWstringLength(){
	// switch on wstring type
	if(IsPtrUsed){
		if(Data == NULL){
			//WstringIterator: GetWstringLength:
			throw ExceptionNULLPtr(L"Text pointer is invalid ", 0, __WFUNCTION__, (void*)Data);
		}
		return Data->size();

	} else {
		return ConstData.size();

	}
}

DLLEXPORT wchar_t Leviathan::StringIterator::GetCurrentCharacter(){

	return GetCharacterAtPos(IteratorPosition);
}

DLLEXPORT wchar_t Leviathan::StringIterator::GetCharacterAtPos(size_t pos){
	// TODO: put a index check here //
	if(IsPtrUsed){

		return (*this->Data)[pos];
	} else {

		return this->ConstData[pos];
	}
}

DLLEXPORT bool Leviathan::StringIterator::MoveToNext(){
	IteratorPosition++;
	// return true if it is still valid //
	if(IsPtrUsed){

		return Data->size() > IteratorPosition;
	} else {

		return ConstData.size() > IteratorPosition;
	}
}

DLLEXPORT void Leviathan::StringIterator::ReInit(wstring* text, bool TakesOwnership /*= false*/){
	// delete old if applicable //
	if(HandlesDelete)
		SAFE_DELETE(Data);

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

DLLEXPORT void Leviathan::StringIterator::ReInit(const wstring& text){
	// delete old if applicable //
	if(HandlesDelete)
		SAFE_DELETE(Data);

	// copied from ctor //
	HandlesDelete = false;
	Data = NULL;

	ConstData = text;

	// start from beginning of string //
	IteratorPosition = 0;

	// set right type //
	IsPtrUsed = false;

	// clear flags //
	CurrentFlags->ClearFlags();
}

DLLEXPORT void Leviathan::StringIterator::StripPreceedingAndTrailingWhitespaceComments(wstring &str){
	// create iterator for finding the right parts //
	StringIterator itr(&str, false);

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

#ifdef _DEBUG
DLLEXPORT void Leviathan::StringIterator::SetDebugMode(const bool &mode){
	DebugMode = true;
}
#endif

// ------------------------------------ //
ITERATORCALLBACK_RETURNTYPE Leviathan::FindFirstQuotedString(StringIterator* instance, Object* IteratorData, int parameters){
	// check is current element a quote //
	wchar_t CurChar(instance->GetCurrentCharacter());

	bool TakeChar = true;
	bool End = false;

	// check for quote //
	QUOTETYPE quotetype = (QUOTETYPE)parameters;

	switch(quotetype){
	case QUOTETYPE_BOTH:
		{
			if(instance->CurrentFlags->IsSet(WSTRINGITERATOR_INSIDE_STRING)){
				// check if we are on the quotes, because we don't want those //
				if(CurChar == L'"' || CurChar == L'\''){
					// if we aren't ignoring special disallow //
					if(!instance->CurrentFlags->IsSet(WSTRINGITERATOR_IGNORE_SPECIAL)){
						TakeChar = false;
					}
				}

			} else {
				End = true;
				TakeChar = false;
			}
		}
	break;
	case QUOTETYPE_SINGLEQUOTES:
		{
			if(instance->CurrentFlags->IsSet(WSTRINGITERATOR_INSIDE_STRING_SINGLE)){
				// check if we are on the quotes, because we don't want those //
				if(CurChar == L'\''){
					// if we aren't ignoring special disallow //
					if(!instance->CurrentFlags->IsSet(WSTRINGITERATOR_IGNORE_SPECIAL)){
						TakeChar = false;
					}
				}

			} else {
				End = true;
				TakeChar = false;
			}
		}
	break;
	case QUOTETYPE_DOUBLEQUOTES:
		{
			if(instance->CurrentFlags->IsSet(WSTRINGITERATOR_INSIDE_STRING_DOUBLE)){
				// check if we are on the quotes, because we don't want those //
				if(CurChar == L'"'){
					// if we aren't ignoring special disallow //
					if(!instance->CurrentFlags->IsSet(WSTRINGITERATOR_IGNORE_SPECIAL)){
						TakeChar = false;
					}
				}

			} else {
				End = true;
				TakeChar = false;
			}
		}
	break;
	}

	IteratorPositionData* tmpdata = static_cast<IteratorPositionData*>(IteratorData);

	if(TakeChar){
		// check is this first quote //
		if(tmpdata->Positions.X == -1){
			// first position! //
			tmpdata->Positions.X = instance->IteratorPosition;
			tmpdata->Positions.Y = instance->IteratorPosition;

		} else {
			// new character can be added //
			tmpdata->Positions.Y = instance->IteratorPosition;
		}

		return ITERATORCALLBACK_RETURNTYPE_CONTINUE;
	}
	if(End){
		// if we have found at least a character we can end this here //
		if(tmpdata->Positions.X != -1){
			return ITERATORCALLBACK_RETURNTYPE_STOP;
		}
	}
	return ITERATORCALLBACK_RETURNTYPE_CONTINUE;
}

Leviathan::ITERATORCALLBACK_RETURNTYPE Leviathan::FindNextNumber(StringIterator* instance, Object* IteratorData, int parameters){
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

ITERATORCALLBACK_RETURNTYPE Leviathan::FindNextNormalCharacterString(StringIterator* instance, Object* IteratorData, int parameters){
	// check is current element a valid element //
	wchar_t CurChar(instance->GetCurrentCharacter());

	bool IsValid = true;

	IteratorPositionData* tmpdata = static_cast<IteratorPositionData*>(IteratorData);

	if((parameters & UNNORMALCHARACTER_TYPE_LOWCODES || parameters & UNNORMALCHARACTER_TYPE_WHITESPACE)
		&& !instance->CurrentFlags->IsSet(WSTRINGITERATOR_INSIDE_STRING))
	{
		if(CurChar <= L' '){
			IsValid = false;
			goto invalidcodelabelunnormalcharacter;
		}
	}

	if(parameters & UNNORMALCHARACTER_TYPE_NON_ASCII){

		if(instance->CurrentFlags->IsSet(WSTRINGITERATOR_INSIDE_STRING) || !(
			(CurChar >= L'0' && CurChar <= L'9') || (CurChar >= L'A' && CurChar <= L'Z') || (CurChar >= L'a' && CurChar <= L'z')))
		{
			IsValid = false;
			goto invalidcodelabelunnormalcharacter;
		}
	}

	if(parameters & UNNORMALCHARACTER_TYPE_CONTROLCHARACTERS && !instance->CurrentFlags->IsSet(WSTRINGITERATOR_INSIDE_STRING)
		&& !instance->CurrentFlags->IsSet(WSTRINGITERATOR_IGNORE_SPECIAL))
	{
		if(((CurChar <= L'/' && CurChar >= L'!') || (CurChar <= L'@' && CurChar >= L':') || (CurChar <= L'`' && CurChar >= L'[')
			|| (CurChar <= L'~' && CurChar >= L'{')) && !(CurChar == L'_' || CurChar == L'-'))
		{
			IsValid = false;
			goto invalidcodelabelunnormalcharacter;
		}
	}

	if(IsValid){
		// check is this first character //
		if(tmpdata->Positions.X == -1){
			// first position! //

			tmpdata->Positions.X = instance->IteratorPosition;
		}

	} else {

invalidcodelabelunnormalcharacter:


		// check for end //
		if(tmpdata->Positions.X != -1){
			// ended //
			tmpdata->Positions.Y = instance->IteratorPosition-1;
			return ITERATORCALLBACK_RETURNTYPE_STOP;
		}
	}

	return ITERATORCALLBACK_RETURNTYPE_CONTINUE;
}

Leviathan::ITERATORCALLBACK_RETURNTYPE Leviathan::FindUntilEquality(StringIterator* instance, Object* IteratorData, int parameters){
	// check is current element a valid element //
	int charvalue((int)instance->GetCurrentCharacter());

	bool IsValid = true;
	bool IsStop = false;

	// what characters are stopping //
	EQUALITYCHARACTER stoptype = (EQUALITYCHARACTER)parameters;

	IteratorAssignmentData* tmpdata = static_cast<IteratorAssignmentData*>(IteratorData);

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

Leviathan::ITERATORCALLBACK_RETURNTYPE Leviathan::FindFromStartUntilCommentOrEnd(StringIterator* instance, Object* IteratorData, int parameters){
	// we can just return if we are inside a string //
	if(instance->CurrentFlags->IsSet(WSTRINGITERATOR_INSIDE_STRING)){
		// position is always valid inside string, goto end for this being valid //
		goto findfromstartuntilcommentorendfuncendlabel;
		//return ITERATORCALLBACK_RETURNTYPE_CONTINUE;
	}
	{
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

Leviathan::ITERATORCALLBACK_RETURNTYPE Leviathan::FindUntilSpecificCharacter(StringIterator* instance, Object* IteratorData, int parameters){
	// get position data //
	IteratorFindUntilData* tmpdata = static_cast<IteratorFindUntilData*>(IteratorData);

	// can this character be added //
	bool ValidChar = true;

	// we can just continue if we are inside a string //
	if(!instance->CurrentFlags->IsSet(WSTRINGITERATOR_INSIDE_STRING)){
		// check did we encounter stop character //
		if(instance->GetCurrentCharacter() == (wchar_t)parameters){
			// skip if ignoring special characters //
			if(!instance->CurrentFlags->IsSet(WSTRINGITERATOR_IGNORE_SPECIAL)){
				// not valid character //
				ValidChar = false;
				// we must have started to encounter the stop character //
				if(tmpdata->Positions.X != -1){
					// we encountered the stop character //
					tmpdata->FoundEnd = true;
				}
			}
		}
	}

	if(ValidChar){
		// valid character set start if not already set //
		if(tmpdata->Positions.X == -1){
			tmpdata->Positions.X = instance->IteratorPosition;
			tmpdata->Positions.Y = tmpdata->Positions.X;
		} else {
			tmpdata->Positions.Y = instance->IteratorPosition;
		}
		return ITERATORCALLBACK_RETURNTYPE_CONTINUE;
	}
	// let's stop if we have found something //
	if(tmpdata->Positions.X != -1){
		return ITERATORCALLBACK_RETURNTYPE_STOP;
	}

	// haven't found anything, we'll need to find something //
	return ITERATORCALLBACK_RETURNTYPE_CONTINUE;
}

Leviathan::ITERATORCALLBACK_RETURNTYPE Leviathan::SkipSomething(StringIterator* instance, Object* notwanted, int parameters){

	// we can just return if we are inside a string //
	if(instance->CurrentFlags->IsSet(WSTRINGITERATOR_INSIDE_STRING)){
		return ITERATORCALLBACK_RETURNTYPE_STOP;
	}
	int curchara = (int)instance->GetCurrentCharacter();
	// check does character match what is skipped //

	UNNORMALCHARACTER stoptype = (UNNORMALCHARACTER)parameters;

	if(parameters & UNNORMALCHARACTER_TYPE_LOWCODES){
		if(curchara <= 32)
			return ITERATORCALLBACK_RETURNTYPE_CONTINUE;
	}

	if(notwanted){
		// Should be a ptr to IteratorCharacterData
		IteratorCharacterData* tmpptr = static_cast<IteratorCharacterData*>(notwanted);

		if(curchara == tmpptr->CharacterToUse){
			// We want to skip it //
			return ITERATORCALLBACK_RETURNTYPE_CONTINUE;
		}
	}

	// didn't match to be skipped characters //
	return ITERATORCALLBACK_RETURNTYPE_STOP;
}
