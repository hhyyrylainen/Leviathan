#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_LINETOKENIZER
#include "LineTokenizer.h"
#endif
using namespace Leviathan;
// ------------------------------------ //


DLLEXPORT int Leviathan::LineTokeNizer::TokeNizeLine(const wstring& str, vector<wstring*> &result){
	int TokenIndex = 0;
	int Level = 0;
	bool InString = false;

	vector<unique_ptr<Int2>> CopyOperations;
	// estimate token count //
	CopyOperations.reserve(3);

	// check how many characters need to be copied to each token //
	for(unsigned int i = 0; i < str.size(); i++){
		// check for chars //
		if(str[i] == L'\"'){
			if(i-1 > -1){
				// check for \ 
				if(str[i-1] == L'\\'){
					// not //

				} else {
					Misc::ToggleBool(InString);
				}
			} else {
				Misc::ToggleBool(InString);
			}
		}
		if(!InString){
			if(str[i] == L'('){
				Level++;
			}
			if(str[i] == L'['){
				Level++;
			}
			if(str[i] == L')'){
				Level--;
			}
			if(str[i] == L']'){
				Level--;
			}
		}
		// check for token end //
		if(Level < 1 && !InString){
			// could break here //
			if(str[i] == L' '){
				// token end //
				TokenIndex++;

				// end previous token just before this one //
				CopyOperations[TokenIndex-1]->Y = i;

				continue;
			}
		}


		// check new token start //
		if((int)CopyOperations.size() < TokenIndex+1){
			// need to add new //
			CopyOperations.push_back(unique_ptr<Int2>(new Int2(i, -1)));
		}
	}
	// reserve space //
	result.clear();
	result.reserve(CopyOperations.size());

	for(unsigned int i = 0; i < CopyOperations.size(); i++){
		// use wstring functions to get sub strings //
		// second is actually lenght of substring (end-start positions) //
		result.push_back(new wstring(str.substr((unsigned int)CopyOperations[i]->X, 
			(unsigned int)CopyOperations[i]->Y-CopyOperations[i]->X)));
	}
	// release memory //
	CopyOperations.clear();

	// no error //
	return 0;
}

DLLEXPORT  int Leviathan::LineTokeNizer::SplitTokenToValues(const wstring& str, vector<wstring> &result){
	// scan the input and create an array of Int2 marking positions //
	vector<unique_ptr<Int2>> CopyOperations;
	// estimate token count //
	CopyOperations.reserve(3);

	bool CurrentStarted = false;

	for(wstring::size_type i = 0; i < str.size(); i++){
		if(!CurrentStarted){
			// check is this proper position //
			if(!Misc::IsCharacterNumber(str[i])){
				// nope, need to skip more characters //
				continue;
			}

			// add new Position marker //
			CopyOperations.push_back(unique_ptr<Int2>(new Int2((int)i, -1)));
			CurrentStarted = true;
		}

		if(!Misc::IsCharacterNumber(str[i])){
currentstringend:

			// current one ended //
			CopyOperations.back()->Y = (int)i;
			// start new one //
			CurrentStarted = false;
			continue;
		}
		// check for final loop run //
		if(!(i+1 < str.length())){
			// little trick to have correct length //
			i++;
			// final one, end the string //
			goto currentstringend;
		}
	}

	// reserve space //
	result.clear();
	result.reserve(CopyOperations.size());

	for(unsigned int i = 0; i < CopyOperations.size(); i++){
		// copy the substring from original string //

		result.push_back(str.substr((unsigned int)CopyOperations[i]->X, /*lenght*/ (unsigned int)(CopyOperations[i]->Y-
			CopyOperations[i]->X)));
	}
	// no error //
	return 0;
}
// ------------------------------------ //
DLLEXPORT int Leviathan::LineTokeNizer::SplitTokenToRTokens(const wstring& str, vector<Token*> &result){

	// loop through all characters and add tokens to the result as required and create sub tokens when necessary //
	bool InString = false;

	vector<unique_ptr<DataForToken>> StringsToCopy;
	Int2 CurrentCharacters(-1);

	vector<Token*> Temporaryresult;

	vector<Token*> TokenTree;
	Token* current = NULL;



	for(wstring::size_type i = 0; i < str.size(); i++){
		// check will current token end //
		if(str[i] == '\"'){
			if(InString)
				InString = false;
			else
				InString = true;
		}
		if(((str[i] == L'[') || (str[i] == L'(')) && (!InString)){
			// a token started, check for parent token //
			if(current != NULL){
				// sub token for this token //
				TokenTree.push_back(current);
			}
			current = new Token();

			// set up copying of characters //
			CurrentCharacters.Y = i-1;
			


			if((CurrentCharacters[0] != CurrentCharacters[1]) && (CurrentCharacters[0] != -1)){
				// add to parent token //
				if(TokenTree.size() == 0){
					// add new //
					TokenTree.push_back(new Token());
					StringsToCopy.push_back(unique_ptr<DataForToken>(new DataForToken(TokenTree.back(), CurrentCharacters)));


				} else {
					// add characters to parent node //
					StringsToCopy.push_back(unique_ptr<DataForToken>(new DataForToken(TokenTree.back(), CurrentCharacters)));
					//TokenTree.back()->GetChangeableData() += tempchars;
					//tempchars.clear();
				}
			}
			CurrentCharacters.SetData(-1);
			// set parent //
			if(TokenTree.size() != 0){
				TokenTree.back()->CreateSubToken(current);
			}
			continue;
		}
		if(((str[i] == L']') || (str[i] == L')')) && (!InString)){
			// current token ended //
			// set text //
			//current->GetChangeableData() += tempchars;
			//tempchars.clear();

			// set up copying of characters //
			CurrentCharacters.Y = i-1;
			if(!((CurrentCharacters[0] == -1) || (CurrentCharacters[0] == CurrentCharacters[1]))){
				StringsToCopy.push_back(unique_ptr<DataForToken>(new DataForToken(current, CurrentCharacters)));
			}
			CurrentCharacters.SetData(-1);

			// add to result //
			Temporaryresult.push_back(current);

			// get last element of tree as current //
			if(TokenTree.size() != 0){
				current = TokenTree.back();
				TokenTree.pop_back();
			}

			continue;
		}
		// check is new copy starting //
		if(CurrentCharacters[0] == -1)
			CurrentCharacters.X = i;

		// add to temporary buffer //
		//tempchars += checkchar;
	}
	// add rest of characters //
	if(CurrentCharacters.X != -1){
		// fix the end index (as final index in string //
		CurrentCharacters.Y = str.length()-1;
		if(current == NULL){
			// check for tree //
			if(TokenTree.size() == 0){
				// needs to add brand new //
				Temporaryresult.push_back(new Token());
				StringsToCopy.push_back(unique_ptr<DataForToken>(new DataForToken(Temporaryresult.back(), CurrentCharacters)));
			} else {
				//TokenTree.back()->GetChangeableData() += tempchars;
				StringsToCopy.push_back(unique_ptr<DataForToken>(new DataForToken(TokenTree.back(), CurrentCharacters)));
			}
		} else {
			//current->GetChangeableData() += tempchars;
			StringsToCopy.push_back(unique_ptr<DataForToken>(new DataForToken(current, CurrentCharacters)));
			//tempchars.clear();
			result.push_back(current);
		}
	}
	// put current //
	if(current != NULL){
		Temporaryresult.push_back(current);
		current = NULL;
	}

	// copy the strings to right places //
	for(unsigned int i = 0; i < StringsToCopy.size(); i++){
		if(StringsToCopy[i]->ToToken->GetData().length() == 0){
			// set as the wstring //
			StringsToCopy[i]->ToToken->SetData(str.substr((unsigned int)StringsToCopy[i]->ToCopyCharacters.X, (unsigned int)(1+
				StringsToCopy[i]->ToCopyCharacters.Y-StringsToCopy[i]->ToCopyCharacters.X)));
		} else {
			// append //
			StringsToCopy[i]->ToToken->GetChangeableData() += str.substr((unsigned int)StringsToCopy[i]->ToCopyCharacters.X, (unsigned int)(1+
				StringsToCopy[i]->ToCopyCharacters.Y-StringsToCopy[i]->ToCopyCharacters.X));
		}
	}
	// clear data //
	StringsToCopy.clear();

	// clear tree //
	for(unsigned int i = 0; i < TokenTree.size(); i++){
		Temporaryresult.push_back(TokenTree[i]);
	}

	// flip the result vector to place highest tokens first //
	std::reverse(Temporaryresult.begin(), Temporaryresult.end());

	// try to create nice structure to final result vector //
	// reserve space for it //
	result.reserve(Temporaryresult.size());

	// try to place all objects of same level in sequential order //
	int SubLevel = -1;
	while(Temporaryresult.size() != 0){
		SubLevel++;
		for(unsigned int i = 0; i < Temporaryresult.size(); i++){
			// check is there already //
			if(Misc::DoesVectorContainValuePointers<Token>(result, Temporaryresult[i])){
				// check child nodes //
				if(SubLevel < 1){
					// can't add sub level stuff, yet //
					continue;
				}

				// is this parent node? //
				if(Temporaryresult[i]->GetParentToken() != NULL){
					// not a parent node, don't start adding this child's //
					continue;
				}

				// loop through sub tokens //
				int CurrentLevel = 0;
				if(TokenRTokenAddSubTokens(CurrentLevel, SubLevel, Temporaryresult[i], result)){
					// all sub tokens are added //
					// remove this and subtokens from the vectors //
					TokenRTokenRemoveAllSubTokensFromVec(Temporaryresult[i], Temporaryresult);

					//Temporaryresult.erase(Temporaryresult.begin()+i);
					//i--;
					i--;
					continue;
				}



			} else {
				// is this a parent node? //
				if(Temporaryresult[i]->GetParentToken() == NULL){
					// it is, can be copied //
					result.push_back(Temporaryresult[i]);
					//Temporaryresult.erase(Temporaryresult.begin()+i);
					//i--;
				}
			}
		}
		// infinity check //
		if(SubLevel > 100){
			// copy rest and it's done //

			for(unsigned int ind = 0; ind < Temporaryresult.size(); ind++){
				if(!Misc::DoesVectorContainValuePointers<Token>(result, Temporaryresult[ind]))
					result.push_back(Temporaryresult[ind]);
			}
			Temporaryresult.clear();
			// add warning //
			Logger::Get()->Info(L"LineTokeNizer: SplitTokenRTokens: failed to correctly copy tokens, iteration over 100", false);
		}
	}


	// no error //
	return 0;
}

bool Leviathan::LineTokeNizer::TokenRTokenAddSubTokens(int curlevel, int maxlevel, Token* curtoken, vector<Token*>& destination){
	// check can this be added //
	if(curlevel <= maxlevel){
		// can be added //
		if(!Misc::DoesVectorContainValuePointers<Token>(destination, curtoken)){
			// add //
			destination.push_back(curtoken);
		}
	} else {
		// can't be added //
		return false;
	}
	if(!(curlevel+1 <= maxlevel)){
		// no further elements can be added //
		if(curtoken->GetSubTokenCount() != 0){
			return false;
		}
		return true;
	}
	// check subs //
	int SubCount = curtoken->GetSubTokenCount();
	bool IsAllSubs = true;
	for(int i = 0; i < SubCount; i++){
		// call this function to use this //
		if(!TokenRTokenAddSubTokens(curlevel+1, maxlevel, curtoken->GetSubToken(i), destination)){
			IsAllSubs = false;
		}
	}
	return IsAllSubs;
}

void Leviathan::LineTokeNizer::TokenRTokenRemoveAllSubTokensFromVec(Token* curtoken, vector<Token*>& destination){
	// remove this //
	for(unsigned int i = 0; i < destination.size(); i++){
		if(destination[i] == curtoken){
			destination.erase(destination.begin()+i);
			i--;
			continue;
		}
	}
	// remove sub tokens //
	int SubCount = curtoken->GetSubTokenCount();
	for(int i = 0; i < SubCount; i++){
		// call this function to use this //
		TokenRTokenRemoveAllSubTokensFromVec(curtoken->GetSubToken(i), destination);
	}
}

// ------------------------------------ //

// ------------------------------------ //

// ------------------------------------ //


