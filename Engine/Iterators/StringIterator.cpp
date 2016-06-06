#include "Include.h"
// ------------------------------------ //
#include "StringIterator.h"

#include "utf8/checked.h"
using namespace Leviathan;
using namespace std;
// ------------------------------------ //
// Defining debug macro //
#if defined(_DEBUG) && (defined(LEVIATHAN_BUILD) || !defined(LEVIATHAN_UE_PLUGIN))
#define ALLOW_DEBUG
#endif

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
DLLEXPORT void Leviathan::StringIterator::ReInit(StringDataIterator* iterator, bool TakesOwnership /*= false*/){
	// Remove the last iterator //
	if(HandlesDelete){

		SAFE_DELETE(DataIterator);
	}

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
Leviathan::ITERATORCALLBACK_RETURNTYPE Leviathan::StringIterator::HandleSpecialCharacters(){
	// check should this special character be ignored //
	if(CurrentFlags & ITERATORFLAG_SET_IGNORE_SPECIAL)
		return ITERATORCALLBACK_RETURNTYPE_CONTINUE;
	
	// check for special characters //
	int character = GetCharacter();

	switch(character){
	case '\\':
		{
			if(!(CurrentFlags & ITERATORFLAG_SET_INSIDE_COMMENT)){
				// ignore next special character //
				CurrentFlags |= ITERATORFLAG_SET_IGNORE_SPECIAL;

#ifdef ALLOW_DEBUG
				if(DebugMode){
					Logger::Get()->Write("Iterator: setting: ITERATORFLAG_SET_IGNORE_SPECIAL");
				}
#endif // _DEBUG
			}
		}
	break;
	case '"':
		{
			// Strings cannot be inside comments //
			if(!(CurrentFlags & ITERATORFLAG_SET_INSIDE_COMMENT)){
				// a string //
				if(!(CurrentFlags & ITERATORFLAG_SET_INSIDE_STRING_DOUBLE)){
#ifdef ALLOW_DEBUG
					if(DebugMode){
						Logger::Get()->Write("Iterator: setting: ITERATORFLAG_SET_INSIDE_STRING_DOUBLE");
					}

					if(DebugMode && !(CurrentFlags & ITERATORFLAG_SET_INSIDE_STRING)){
						Logger::Get()->Write("Iterator: setting: ITERATORFLAG_SET_INSIDE_STRING");
					}
#endif // _DEBUG
					// set //
					CurrentFlags |= ITERATORFLAG_SET_INSIDE_STRING_DOUBLE;

					// set as inside string //
					CurrentFlags |= ITERATORFLAG_SET_INSIDE_STRING;

				} else {
#ifdef ALLOW_DEBUG
					if(DebugMode){
						Logger::Get()->Write("Iterator: set flag end: ITERATORFLAG_SET_INSIDE_STRING_DOUBLE");
					}
#endif // _DEBUG
					// set ending flag //
					CurrentFlags |= ITERATORFLAG_SET_INSIDE_STRING_DOUBLE_END;
				}
			}
		}
	break;
	case '\'':
		{
			// Strings cannot be inside comments //
			if(!(CurrentFlags & ITERATORFLAG_SET_INSIDE_COMMENT)){
				// a string //
				if(!(CurrentFlags & ITERATORFLAG_SET_INSIDE_STRING_SINGLE)){
#ifdef ALLOW_DEBUG
					if(DebugMode){
						Logger::Get()->Write("Iterator: setting: ITERATORFLAG_SET_INSIDE_STRING_SINGLE");
					}

					if(DebugMode && !(CurrentFlags & ITERATORFLAG_SET_INSIDE_STRING)){
						Logger::Get()->Write("Iterator: setting: ITERATORFLAG_SET_INSIDE_STRING");
					}
#endif // _DEBUG
					// set //
					CurrentFlags |= ITERATORFLAG_SET_INSIDE_STRING_SINGLE;

					// set as inside string //
					CurrentFlags |= ITERATORFLAG_SET_INSIDE_STRING;

				} else {
#ifdef ALLOW_DEBUG
					if(DebugMode){
						Logger::Get()->Write("Iterator: set flag end: ITERATORFLAG_SET_INSIDE_STRING_SINGLE");
					}
#endif // _DEBUG

					CurrentFlags |= ITERATORFLAG_SET_INSIDE_STRING_SINGLE_END;
				}
			}
		}
		break;
	case '/':
		{
			// Comments cannot be inside strings //
			if(!(CurrentFlags & ITERATORFLAG_SET_INSIDE_STRING)){
				// There might be a comment beginning //
				int nextchar = GetCharacter(1);

				if(nextchar == '/'){
					// C++-style comment starts //
					if(!(CurrentFlags & ITERATORFLAG_SET_INSIDE_CPPCOMMENT)){
#ifdef ALLOW_DEBUG
						if(DebugMode){
							Logger::Get()->Write("Iterator: setting: ITERATORFLAG_SET_INSIDE_CPPCOMMENT");
						}

						if(DebugMode && !(CurrentFlags & ITERATORFLAG_SET_INSIDE_COMMENT)){
							Logger::Get()->Write("Iterator: setting: ITERATORFLAG_SET_INSIDE_COMMENT");
						}
#endif // _DEBUG
						CurrentFlags |= ITERATORFLAG_SET_INSIDE_CPPCOMMENT;
						CurrentFlags |= ITERATORFLAG_SET_INSIDE_COMMENT;
					}


				} else if(nextchar == '*'){
					// C-style comment starts //
					if(!(CurrentFlags & ITERATORFLAG_SET_INSIDE_CCOMMENT)){
#ifdef ALLOW_DEBUG
						if(DebugMode){
							Logger::Get()->Write("Iterator: setting: ITERATORFLAG_SET_INSIDE_CCOMMENT");
						}

						if(DebugMode && !(CurrentFlags & ITERATORFLAG_SET_INSIDE_COMMENT)){
							Logger::Get()->Write("Iterator: setting: ITERATORFLAG_SET_INSIDE_COMMENT");
						}
#endif // _DEBUG
						CurrentFlags |= ITERATORFLAG_SET_INSIDE_CCOMMENT;
						CurrentFlags |= ITERATORFLAG_SET_INSIDE_COMMENT;
					}

				} else if(CurrentFlags & ITERATORFLAG_SET_INSIDE_CCOMMENT){
					// C-style comment might end //

					int previouschar = GetPreviousCharacter();

					if(previouschar == '*'){

						// Set as ending //
						CurrentFlags |= ITERATORFLAG_SET_CCOMMENT_END;
#ifdef ALLOW_DEBUG
						if(DebugMode){
							Logger::Get()->Write("Iterator: set flag end: ITERATORFLAG_SET_CCOMMENT_END");
						}
#endif // _DEBUG
					}
				}
			}

		}
		break;
	case '\n':
		{
			// A C++-style comment might end //
			if(CurrentFlags & ITERATORFLAG_SET_INSIDE_CPPCOMMENT){
				// Set as ending //
				CurrentFlags |= ITERATORFLAG_SET_CPPCOMMENT_END;
#ifdef ALLOW_DEBUG
				if(DebugMode){
					Logger::Get()->Write("Iterator: set flag end: ITERATORFLAG_SET_CPPCOMMENT_END");
				}
#endif // _DEBUG
			}
		}
		break;
	}

	return ITERATORCALLBACK_RETURNTYPE_CONTINUE;
}

Leviathan::ITERATORCALLBACK_RETURNTYPE Leviathan::StringIterator::CheckActiveFlags(){
	if(CurrentFlags & ITERATORFLAG_SET_STOP)
		return ITERATORCALLBACK_RETURNTYPE_STOP;

	// Reset 1 character long flags //
	if(CurrentFlags & ITERATORFLAG_SET_IGNORE_SPECIAL){
#ifdef ALLOW_DEBUG
		if(DebugMode){
			Logger::Get()->Write("Iterator: flag: STRINGITERATOR_IGNORE_SPECIAL");
		}
#endif // _DEBUG

		// check should end now //
		if(CurrentFlags & ITERATORFLAG_SET_IGNORE_SPECIAL_END){
#ifdef ALLOW_DEBUG
			if(DebugMode){
				Logger::Get()->Write("Iterator: flag ended: STRINGITERATOR_IGNORE_SPECIAL");
			}
#endif // _DEBUG
			// unset both //
			CurrentFlags &= ~ITERATORFLAG_SET_IGNORE_SPECIAL_END;
			CurrentFlags &= ~ITERATORFLAG_SET_IGNORE_SPECIAL;

		} else {
#ifdef ALLOW_DEBUG
			if(DebugMode){
				Logger::Get()->Write("Iterator: flag ends next character: ITERATORFLAG_SET_IGNORE_SPECIAL");
			}
#endif // _DEBUG
			// set to end next character //
			CurrentFlags |= ITERATORFLAG_SET_IGNORE_SPECIAL_END;
		}
	}

	// reset end flags before we process this cycle further //
	if(CurrentFlags & ITERATORFLAG_SET_INSIDE_STRING_DOUBLE_END){
#ifdef ALLOW_DEBUG
		if(DebugMode){
			Logger::Get()->Write("Iterator: flag ends: ITERATORFLAG_SET_INSIDE_STRING_DOUBLE");
		}
#endif // _DEBUG
		// unset flag //
		CurrentFlags &= ~ITERATORFLAG_SET_INSIDE_STRING_DOUBLE;
		CurrentFlags &= ~ITERATORFLAG_SET_INSIDE_STRING_DOUBLE_END;
	}

	if(CurrentFlags & ITERATORFLAG_SET_INSIDE_STRING_SINGLE_END){
#ifdef ALLOW_DEBUG
		if(DebugMode){
			Logger::Get()->Write("Iterator: flag ends: ITERATORFLAG_SET_INSIDE_STRING_SINGLE");
		}
#endif // _DEBUG
		// unset flag //
		CurrentFlags &= ~ITERATORFLAG_SET_INSIDE_STRING_SINGLE;
		CurrentFlags &= ~ITERATORFLAG_SET_INSIDE_STRING_SINGLE_END;
	}

	// Check can we unset the whole string flag //
	if(CurrentFlags & ITERATORFLAG_SET_INSIDE_STRING){
		if(!(CurrentFlags & ITERATORFLAG_SET_INSIDE_STRING_DOUBLE) && !(CurrentFlags & ITERATORFLAG_SET_INSIDE_STRING_SINGLE)){
	#ifdef ALLOW_DEBUG
			if(DebugMode){
				Logger::Get()->Write("Iterator: flag ends: ITERATORFLAG_SET_INSIDE_STRING");
			}
	#endif // _DEBUG
			// can unset this //
			CurrentFlags &= ~ITERATORFLAG_SET_INSIDE_STRING;
		}
	}

	// Unsetting comment flags //
	if(CurrentFlags & ITERATORFLAG_SET_CPPCOMMENT_END){
#ifdef ALLOW_DEBUG
		if(DebugMode){
			Logger::Get()->Write("Iterator: flag ends: ITERATORFLAG_SET_CPPCOMMENT_END");
		}
#endif // _DEBUG
		// unset flag //
		CurrentFlags &= ~ITERATORFLAG_SET_CPPCOMMENT_END;
		CurrentFlags &= ~ITERATORFLAG_SET_INSIDE_CPPCOMMENT;
	}

	// C-style flag //
	if(CurrentFlags & ITERATORFLAG_SET_CCOMMENT_END){
#ifdef ALLOW_DEBUG
		if(DebugMode){
			Logger::Get()->Write("Iterator: flag ends: ITERATORFLAG_SET_CCOMMENT_END");
		}
#endif // _DEBUG
		// unset flag //
		CurrentFlags &= ~ITERATORFLAG_SET_CCOMMENT_END;
		CurrentFlags &= ~ITERATORFLAG_SET_INSIDE_CCOMMENT;
	}

	// Check can we unset the whole comment flag //
	if(CurrentFlags & ITERATORFLAG_SET_INSIDE_COMMENT){
		if(!(CurrentFlags & ITERATORFLAG_SET_INSIDE_CPPCOMMENT) && !(CurrentFlags & ITERATORFLAG_SET_INSIDE_CCOMMENT)){
#ifdef ALLOW_DEBUG
			if(DebugMode){
				Logger::Get()->Write("Iterator: flag ends: ITERATORFLAG_SET_INSIDE_COMMENT");
			}
#endif // _DEBUG
			// can unset this //
			CurrentFlags &= ~ITERATORFLAG_SET_INSIDE_COMMENT;
		}
	}


	return ITERATORCALLBACK_RETURNTYPE_CONTINUE;
}
// ------------------------------------ //
#ifdef ALLOW_DEBUG
DLLEXPORT void Leviathan::StringIterator::SetDebugMode(const bool &mode){
	DebugMode = true;
}
#endif

DLLEXPORT size_t Leviathan::StringIterator::GetLastValidCharIndex() const{
	return DataIterator->GetLastValidIteratorPosition();
}
// ------------------------------------ //
DLLEXPORT int Leviathan::StringIterator::GetCharacter(size_t forward /*= 0*/){
	// Special case for current character //
	if(!forward){

		if(!CurrentStored){

			DataIterator->GetNextCharCode(CurrentCharacter, 0);
			CurrentStored = true;
		}

		return CurrentCharacter;
	}

	// Get the character from our iterator and store it to a temporary value and then return it //
	int tmpval = -1;
	DataIterator->GetNextCharCode(tmpval, forward);

	return tmpval;
}

DLLEXPORT int Leviathan::StringIterator::GetPreviousCharacter(){
	int tmpval = -1;
	if(!DataIterator->GetPreviousCharacter(tmpval)){
		// Darn //
		return 0;
	}
	return tmpval;
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::StringIterator::MoveToNext(){
	DataIterator->MoveToNextCharacter();
	bool valid = DataIterator->IsPositionValid();
	// It's important to reset this //
	CurrentStored = false;

	// We need to handle the flags on this position if we aren't on the first character //
	if(valid && DataIterator->CurrentIteratorPosition() != 0){
#ifdef ALLOW_DEBUG
		if(DebugMode){
			Logger::Get()->Write("Iterator: user move: to next");
			Logger::Get()->Write("Iterator: handle: CheckActiveFlags, HandleSpecialCharacters");
		}
#endif // _DEBUG

		CheckActiveFlags();

		// check current character //
		HandleSpecialCharacters();
	}

	return valid;
}

DLLEXPORT size_t Leviathan::StringIterator::GetPosition(){
	return DataIterator->CurrentIteratorPosition();
}

DLLEXPORT size_t Leviathan::StringIterator::GetCurrentLine(){
	return DataIterator->GetCurrentLineNumber();
}

DLLEXPORT bool Leviathan::StringIterator::IsOutOfBounds(){
	return !DataIterator->IsPositionValid();
}
// ------------------ Iterating functions ------------------ //
#ifdef ALLOW_DEBUG
#define ITR_FUNCDEBUG(x) {\
	if(DebugMode){\
	Logger::Get()->Write("Iterator: procfunc: " + std::string(x));\
	}\
}
#else
#define ITR_FUNCDEBUG(x) {}
#endif // _DEBUG




Leviathan::ITERATORCALLBACK_RETURNTYPE Leviathan::StringIterator::FindFirstQuotedString(IteratorPositionData* data,
    QUOTETYPE quotes,  int specialflags)
{

	int currentcharacter = GetCharacter();

	bool TakeChar = true;
	bool End = false;


	if(currentcharacter == '\n' && specialflags & SPECIAL_ITERATOR_ONNEWLINE_STOP){
		ITR_FUNCDEBUG("Stopping to new line");

		if(data->Positions.Start){
			// Set the last character to two before this (skip the current and " and end there) //
			int previouscheck = GetPreviousCharacter();
			
			// Check do we need to go back 2 characters or just one //
			if((quotes == QUOTETYPE_BOTH && (previouscheck == '"' || previouscheck == '\'')) || 
				(quotes == QUOTETYPE_DOUBLEQUOTES && previouscheck == '"') || (quotes == QUOTETYPE_SINGLEQUOTES && previouscheck == '\''))
			{
				ITR_FUNCDEBUG("Going back over an extra quote character");
				data->Positions.End = GetPosition()-2;
			} else {
				data->Positions.End = GetPosition()-1;
			}

			ITR_FUNCDEBUG("Ending to new line, end is now: "+Convert::ToString(data->Positions.End));
		}

		MoveToNext();
		return ITERATORCALLBACK_RETURNTYPE_STOP;
	}

	switch(quotes){
	case QUOTETYPE_BOTH:
		{
			if(CurrentFlags & ITERATORFLAG_SET_INSIDE_STRING){
				// check if we are on the quotes, because we don't want those //
				if(currentcharacter == '"' || currentcharacter == '\''){
					// if we aren't ignoring special disallow //
					if(!(CurrentFlags & ITERATORFLAG_SET_IGNORE_SPECIAL)){
						TakeChar = false;
						ITR_FUNCDEBUG("Found quote character");
					}
				}

			} else {
				End = true;
				ITR_FUNCDEBUG("Outside quotes");
			}
		}
		break;
	case QUOTETYPE_SINGLEQUOTES:
		{
			if(CurrentFlags & ITERATORFLAG_SET_INSIDE_STRING_SINGLE){
				// check if we are on the quotes, because we don't want those //
				if(currentcharacter == '\''){
					// if we aren't ignoring special disallow //
					if(!(CurrentFlags & ITERATORFLAG_SET_IGNORE_SPECIAL)){
						TakeChar = false;
						ITR_FUNCDEBUG("Found quote character");
					}
				}

			} else {
				End = true;
				ITR_FUNCDEBUG("Outside quotes");
			}
		}
		break;
	case QUOTETYPE_DOUBLEQUOTES:
		{
			if(CurrentFlags & ITERATORFLAG_SET_INSIDE_STRING_DOUBLE){
				// check if we are on the quotes, because we don't want those //
				if(currentcharacter == '"'){
					// if we aren't ignoring special disallow //
					if(!(CurrentFlags & ITERATORFLAG_SET_IGNORE_SPECIAL)){
						TakeChar = false;
						ITR_FUNCDEBUG("Found quote character");
					}
				}

			} else {
				End = true;
				ITR_FUNCDEBUG("Outside quotes");
			}
		}
		break;
	}

	// If we have found a character this is on the ending quote //
	if(!TakeChar && data->Positions.Start){
		
		// Set the last character to the one before this (skip the " and end there) //
		data->Positions.End = GetPosition() - 1;
		ITR_FUNCDEBUG("On ending quote, end is now: "+Convert::ToString(data->Positions.End));
	}

	if(End){
		// if we have found at least a character we can end this here //
		if(data->Positions.Start){
			// Set the last character to two before this (skip the current and " and end there) //
			data->Positions.End = GetPosition() - 2;
			ITR_FUNCDEBUG("Ending outside quotes, end is now: "+Convert::ToString(data->Positions));
			return ITERATORCALLBACK_RETURNTYPE_STOP;
		}
	} else if(TakeChar){
		// check is this first quoted character //
		if(!data->Positions.Start){
			// first position! //
			data->Positions.Start = GetPosition();
			ITR_FUNCDEBUG("First character found: "+Convert::ToString(data->Positions.Start));

		}
	}

	return ITERATORCALLBACK_RETURNTYPE_CONTINUE;
}

Leviathan::ITERATORCALLBACK_RETURNTYPE Leviathan::StringIterator::FindNextNormalCharacterString(
    IteratorPositionData* data, int stopflags, int specialflags)
{

	int currentcharacter = GetCharacter();

	bool IsValid = true;

	if(currentcharacter == '\n' && specialflags & SPECIAL_ITERATOR_ONNEWLINE_STOP){
		ITR_FUNCDEBUG("Stopping to new line");

		if(data->Positions.Start){
			// ended //
			data->Positions.End = GetPosition() - 1;
			ITR_FUNCDEBUG("Ending to new line, end is now: "+Convert::ToString(data->Positions.End));
		}

		MoveToNext();
		return ITERATORCALLBACK_RETURNTYPE_STOP;
	}


	// If set this is invalid inside comments //
	if((specialflags & SPECIAL_ITERATOR_HANDLECOMMENTS_ASSTRING) && (CurrentFlags & ITERATORFLAG_SET_INSIDE_COMMENT)){
		IsValid = false;
		goto invalidcodelabelunnormalcharacter;
	}


	if((stopflags & UNNORMALCHARACTER_TYPE_LOWCODES || stopflags & UNNORMALCHARACTER_TYPE_WHITESPACE)
		&& !(CurrentFlags & ITERATORFLAG_SET_INSIDE_STRING))
	{
		if(currentcharacter <= ' '){
			IsValid = false;
			goto invalidcodelabelunnormalcharacter;
		}
	}

	if(stopflags & UNNORMALCHARACTER_TYPE_NON_ASCII){

		if(CurrentFlags & ITERATORFLAG_SET_INSIDE_STRING || !(
			(currentcharacter >= '0' && currentcharacter <= '9') || (currentcharacter >= 'A' && currentcharacter <= 'Z') || 
			(currentcharacter >= 'a' && currentcharacter <= 'z')))
		{
			IsValid = false;
			goto invalidcodelabelunnormalcharacter;
		}
	}

	if(stopflags & UNNORMALCHARACTER_TYPE_CONTROLCHARACTERS && !(CurrentFlags & ITERATORFLAG_SET_INSIDE_STRING)
		&& !(CurrentFlags & ITERATORFLAG_SET_IGNORE_SPECIAL))
	{
		if(((currentcharacter <= '/' && currentcharacter >= '!') || (currentcharacter <= '@' && currentcharacter >= ':') 
			|| (currentcharacter <= '`' && currentcharacter >= '[')	|| (currentcharacter <= '~' && currentcharacter >= '{')) 
			&& !(currentcharacter == '_' || currentcharacter == '-'))
		{
			IsValid = false;
			goto invalidcodelabelunnormalcharacter;
		}
	}

	if(IsValid){
		// check is this first character //
		if(!data->Positions.Start){
			// first position! //
			data->Positions.Start = GetPosition();
			ITR_FUNCDEBUG("Started: "+Convert::ToString(data->Positions.Start));
		}

	} else {

invalidcodelabelunnormalcharacter:


		// check for end //
		if(data->Positions.Start){
			// ended //
			data->Positions.End = GetPosition() - 1;
			ITR_FUNCDEBUG("End now: "+Convert::ToString(data->Positions.End));
			return ITERATORCALLBACK_RETURNTYPE_STOP;
		}
	}

	return ITERATORCALLBACK_RETURNTYPE_CONTINUE;
}

Leviathan::ITERATORCALLBACK_RETURNTYPE Leviathan::StringIterator::FindNextNumber(IteratorNumberFindData* data, 
    DECIMALSEPARATORTYPE decimal, int specialflags)
{
	// Check is the current element a part of a number //

	int currentcharacter = GetCharacter();

	bool IsValid = false;

	if(currentcharacter == '\n' && specialflags & SPECIAL_ITERATOR_ONNEWLINE_STOP){
		ITR_FUNCDEBUG("Stopping to new line");

		if(data->Positions.Start){
			// ended //
			data->Positions.End = GetPosition() - 1;
			ITR_FUNCDEBUG("Ending to new line, end is now: "+Convert::ToString(data->Positions.End));
		}

		MoveToNext();
		return ITERATORCALLBACK_RETURNTYPE_STOP;
	}

	// Comments might be skipped //
	if(!(specialflags & SPECIAL_ITERATOR_HANDLECOMMENTS_ASSTRING) || !(CurrentFlags & ITERATORFLAG_SET_INSIDE_COMMENT)){

		if((currentcharacter >= 48) && (currentcharacter <= 57)){
			// Is a plain old digit //
			IsValid = true;

		} else {
			// Check is it a decimal separator (1 allowed) or a negativity sign in front //
			if(currentcharacter == '+' || currentcharacter == '-'){

				if((data->DigitsFound < 1) && (!data->NegativeFound)){
					IsValid = true;
				}
				data->NegativeFound = true;
			} else if (((currentcharacter == '.') && ((decimal == DECIMALSEPARATORTYPE_DOT) || (decimal == DECIMALSEPARATORTYPE_BOTH))) ||
				((currentcharacter == ',') && ((decimal == DECIMALSEPARATORTYPE_COMMA) || (decimal == DECIMALSEPARATORTYPE_BOTH))))
			{
				if((!data->DecimalFound) && (data->DigitsFound > 0)){
					IsValid = true;
					data->DecimalFound = true;
				}
			}
		}
	} else {
		ITR_FUNCDEBUG("Ignoring inside a comment");
	}

	if(IsValid){
		// check is this first digit //
		data->DigitsFound++;
		if(!data->Positions.Start){
			// first position! //

			data->Positions.Start = GetPosition();
			ITR_FUNCDEBUG("Data started: "+Convert::ToString(data->Positions.Start));
		}

	} else {
		// check for end //
		if(data->Positions.Start){
			// ended //
			data->Positions.End = GetPosition() - 1;
			ITR_FUNCDEBUG("End now: "+Convert::ToString(data->Positions.End));
			return ITERATORCALLBACK_RETURNTYPE_STOP;
		}

	}
	return ITERATORCALLBACK_RETURNTYPE_CONTINUE;
}

Leviathan::ITERATORCALLBACK_RETURNTYPE Leviathan::StringIterator::FindUntilEquality(IteratorAssignmentData* data, 
    EQUALITYCHARACTER equality, int specialflags)
{
	// check is current element a valid element //
	int charvalue = GetCharacter();

	bool IsValid = true;
	bool IsStop = false;

	if(charvalue == '\n' && specialflags & SPECIAL_ITERATOR_ONNEWLINE_STOP){
		ITR_FUNCDEBUG("Stopping to new line");

		MoveToNext();
		return ITERATORCALLBACK_RETURNTYPE_STOP;
	}

	// Comments cannot be part of this //
	if((specialflags & SPECIAL_ITERATOR_HANDLECOMMENTS_ASSTRING) && (CurrentFlags & ITERATORFLAG_SET_INSIDE_COMMENT)){
		// Not valid inside a comment //
		ITR_FUNCDEBUG("Comment skipped");
		IsValid = false;

	} else {

		// Skip if this is a space //
		if(charvalue < 33){
			// Not allowed in a name //
			ITR_FUNCDEBUG("Whitespace skipped");
			IsValid = false;
		}

		if(equality == EQUALITYCHARACTER_TYPE_ALL){
			// check for all possible value separators //
			if(charvalue == '=' || charvalue == ':'){

				if(!(CurrentFlags & ITERATORFLAG_SET_IGNORE_SPECIAL)){
					// If ignored don't stop //
					ITR_FUNCDEBUG("Found = or :");
					IsStop = true;
				}
			}
		} else if(equality == EQUALITYCHARACTER_TYPE_EQUALITY){
			// Check for an equality sign //
			if(charvalue == '='){
				if(!(CurrentFlags & ITERATORFLAG_SET_IGNORE_SPECIAL)){
					ITR_FUNCDEBUG("Found =");
					IsStop = true;
				}
			}
		} else if (equality == EQUALITYCHARACTER_TYPE_DOUBLEDOTSTYLE){
			// Check does it match the characters //
			if(charvalue == ':'){
				if(!(CurrentFlags & ITERATORFLAG_SET_IGNORE_SPECIAL)){
					ITR_FUNCDEBUG("Found :");
					IsStop = true;
				}
			}
		}
	}

	if(!IsStop){
		// end if end already found //
		if(data->SeparatorFound){
			ITR_FUNCDEBUG("Found stop");
			return ITERATORCALLBACK_RETURNTYPE_STOP;
		}
	} else {
		data->SeparatorFound = true;
		IsValid = false;
	}

	if(IsValid){
		// Check is this the first character //
		if(!data->Positions.Start){
			// first position! //
			data->Positions.Start = GetPosition();
			ITR_FUNCDEBUG("Data started: "+Convert::ToString(data->Positions.Start));

		} else {
			// Set end to this valid character //
			data->Positions.End = GetPosition();
			ITR_FUNCDEBUG("End now: "+Convert::ToString(data->Positions.End));
		}
	}


	return ITERATORCALLBACK_RETURNTYPE_CONTINUE;
}

DLLEXPORT ITERATORCALLBACK_RETURNTYPE Leviathan::StringIterator::SkipSomething(IteratorCharacterData &data, 
    const int additionalskip, const int specialflags)
{

	int curchara = GetCharacter();

	if(curchara == '\n' && specialflags & SPECIAL_ITERATOR_ONNEWLINE_STOP){
		ITR_FUNCDEBUG("Stopping to new line");

		MoveToNext();
		return ITERATORCALLBACK_RETURNTYPE_STOP;
	}


	// We can probably always skip inside a comment //
	if((specialflags & SPECIAL_ITERATOR_HANDLECOMMENTS_ASSTRING) && (CurrentFlags & ITERATORFLAG_SET_INSIDE_COMMENT)){

		return ITERATORCALLBACK_RETURNTYPE_CONTINUE;
	}

	// We can just return if we are inside a string //
	if(CurrentFlags & ITERATORFLAG_SET_INSIDE_STRING){
		return ITERATORCALLBACK_RETURNTYPE_STOP;
	}

	// cCheck does the character match what is being skipped //

	if(additionalskip & UNNORMALCHARACTER_TYPE_LOWCODES){
		if(curchara <= 32)
			return ITERATORCALLBACK_RETURNTYPE_CONTINUE;
	}

	if(curchara == data.CharacterToUse){
		// We want to skip it //
		return ITERATORCALLBACK_RETURNTYPE_CONTINUE;
	}

	// didn't match to be skipped characters //
	return ITERATORCALLBACK_RETURNTYPE_STOP;
}

Leviathan::ITERATORCALLBACK_RETURNTYPE Leviathan::StringIterator::FindUntilSpecificCharacter(
    IteratorFindUntilData* data, int character, int specialflags)
{

	// Can this character be added //
	bool ValidChar = true;

	int tmpchara = GetCharacter();

	if(tmpchara == '\n' && specialflags & SPECIAL_ITERATOR_ONNEWLINE_STOP){
		ITR_FUNCDEBUG("Stopping to new line");

		if(data->Positions.Start){
			// This should be fine to get here //
			data->Positions.End = GetPosition() - 1;
			ITR_FUNCDEBUG("Ending to new line, end is now: "+Convert::ToString(data->Positions.End));
		}

		MoveToNext();
		return ITERATORCALLBACK_RETURNTYPE_STOP;
	}

	// We can just continue if we are inside a string //
	if(!(CurrentFlags & ITERATORFLAG_SET_INSIDE_STRING) && !((specialflags & SPECIAL_ITERATOR_HANDLECOMMENTS_ASSTRING) 
		&& (CurrentFlags & ITERATORFLAG_SET_INSIDE_COMMENT)))
	{
		// Check did we encounter stop character //
        if (DebugMode) {
            
            std::string value = "Trying to match: ";
            utf8::append(tmpchara, std::back_inserter(value));
            value += "==" + Convert::ToString(tmpchara);

            Logger::Get()->Write("Iterator: procfunc: " + value); \
        }

		if(tmpchara == character){
			// Skip if ignoring special characters //
			if(!(CurrentFlags & ITERATORFLAG_SET_IGNORE_SPECIAL)){
				// Not valid character //
				ValidChar = false;
				ITR_FUNCDEBUG("Found match");
				// We must have started to encounter the stop character //
				if(data->Positions.Start){
					// We encountered the stop character //
					data->FoundEnd = true;
					ITR_FUNCDEBUG("Encountered end");
				}
			}
		}
	} else {
#ifdef ALLOW_DEBUG
		if((CurrentFlags & ITERATORFLAG_SET_INSIDE_STRING)){
			ITR_FUNCDEBUG("Ignoring inside string");
		}
		if((specialflags & SPECIAL_ITERATOR_HANDLECOMMENTS_ASSTRING) &&
            (CurrentFlags & ITERATORFLAG_SET_INSIDE_COMMENT))
        {
			ITR_FUNCDEBUG("Ignoring inside comment");
		}
#endif // _DEBUG
	}

	if(ValidChar){
		// valid character set start if not already set //
		if(!data->Positions.Start){
			data->Positions.Start = GetPosition();
			data->Positions.End = data->Positions.Start;
			ITR_FUNCDEBUG("Data started: "+Convert::ToString(data->Positions.Start));
		}
		return ITERATORCALLBACK_RETURNTYPE_CONTINUE;
	}
	// let's stop if we have found something //
	if(data->Positions.Start){
		// This should be fine to get here //
		data->Positions.End = GetPosition() - 1;
		ITR_FUNCDEBUG("Ending here: "+Convert::ToString(data->Positions.End));
		return ITERATORCALLBACK_RETURNTYPE_STOP;
	}

	// haven't found anything, we'll need to find something //
	return ITERATORCALLBACK_RETURNTYPE_CONTINUE;
}

DLLEXPORT ITERATORCALLBACK_RETURNTYPE Leviathan::StringIterator::FindUntilNewLine(IteratorFindUntilData* data){
	// Continue if the current character is a new line character //
	int curcharacter = GetCharacter();

	bool winmulti = false;

	// Ignore if ignoring special characters //
	if(CurrentFlags & ITERATORFLAG_SET_IGNORE_SPECIAL){

		goto positionisvalidlabelstringiteratorfindnewline;
	}


	if(curcharacter == '\r' && GetCharacter(1) == '\n')
		winmulti = true;

	// All line separator characters should be here //
	if(curcharacter == '\r' || curcharacter == '\n' || winmulti || 
        // Unicode newlines //
        curcharacter == 0x0085 || curcharacter == 0x2028 || curcharacter == 0x2029){

		if(!data->FoundEnd){
			// Ignore the first new line //
			data->FoundEnd = true;
			ITR_FUNCDEBUG("Ignoring first newline character");
			goto positionisvalidlabelstringiteratorfindnewline;
		}

		size_t useendpos = GetPosition()-1;

		// This is a new line character //
		if(winmulti){
			// Two characters have to be skipped \r\n //
			MoveToNext();
			MoveToNext();

		} else {
			// Move out of the newline character //
			MoveToNext();
		}
		ITR_FUNCDEBUG("Found newline character");

		// End before this character //
		data->Positions.End = useendpos;
		ITR_FUNCDEBUG("Ending here: "+Convert::ToString(data->Positions.End));

		return ITERATORCALLBACK_RETURNTYPE_STOP;
	}

positionisvalidlabelstringiteratorfindnewline:


	// Set position //
	if(!data->Positions.Start){

		// End before this character //
		data->Positions.Start = GetPosition();
		data->FoundEnd = true;
		ITR_FUNCDEBUG("Data started: "+Convert::ToString(data->Positions.Start));
	}

	return ITERATORCALLBACK_RETURNTYPE_CONTINUE;
}

DLLEXPORT ITERATORCALLBACK_RETURNTYPE StringIterator::FindInMatchingParentheses(
    IteratorNestingLevelData* data, int left, int right, int specialflags)
{
	// Ignore if ignoring special characters //
	if(!(CurrentFlags & ITERATORFLAG_SET_IGNORE_SPECIAL) &&
        !(CurrentFlags & ITERATORFLAG_SET_INSIDE_STRING))
    {

        int currentcharacter = GetCharacter();
        
        if(currentcharacter == '\n' && specialflags & SPECIAL_ITERATOR_ONNEWLINE_STOP){

            // Invalid, always //
            return ITERATORCALLBACK_RETURNTYPE_STOP;
        }

        // Nesting level starts //
        if(currentcharacter == left){

            ++data->NestingLevel;

            if(data->NestingLevel > 1){

                // There where multiple lefts in a row, like "[[[...]]]"
                goto isinsidevalidleftrightpair;
            }
            
            return ITERATORCALLBACK_RETURNTYPE_CONTINUE;
        }

        // One nesting level is ending //
        if(currentcharacter == right){

            --data->NestingLevel;

            if(data->NestingLevel == 0){

                data->Positions.End = GetPosition() - 1;
                return ITERATORCALLBACK_RETURNTYPE_STOP;
            }
        }
	}

isinsidevalidleftrightpair:

    if(!data->Positions.Start && data->NestingLevel > 0){

        data->Positions.Start = GetPosition();
    }

    return ITERATORCALLBACK_RETURNTYPE_CONTINUE;
}
