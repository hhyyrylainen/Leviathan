#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_MISC
#include "Misc.h"
#include "Define.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
#include "Logger.h"
#include "Convert.h"
#include "DataBlock.h"
#ifdef _DEBUG
#include "TimingMonitor.h"
#endif // _DEBUG

//// string conversions
wstring Misc::EmptyString = L"";


__int64 Misc::GetTimeMs64()
{

		/* Windows */
	FILETIME ft;
	LARGE_INTEGER li;

		/* Get the amount of 100 nano seconds intervals elapsed since January 1, 1601 (UTC) and copy it
		* to a LARGE_INTEGER structure. */
	GetSystemTimeAsFileTime(&ft);
	li.LowPart = ft.dwLowDateTime;
	li.HighPart = ft.dwHighDateTime;

	__int64 ret = li.QuadPart;
	ret -= 116444736000000000LL; /* Convert from file time to UNIX epoch time. */
	ret /= 10000; /* From 100 nano seconds (10^-7) to 1 millisecond (10^-3) intervals */

	return ret;
}
__int64 Misc::GetTimeMicro64()
{

		/* Windows */
	FILETIME ft;
	LARGE_INTEGER li;

		/* Get the amount of 100 nano seconds intervals elapsed since January 1, 1601 (UTC) and copy it
		* to a LARGE_INTEGER structure. */
	GetSystemTimeAsFileTime(&ft);
	li.LowPart = ft.dwLowDateTime;
	li.HighPart = ft.dwHighDateTime;

	__int64 ret = li.QuadPart;
	ret -= 116444736000000000LL; /* Convert from file time to UNIX epoch time. */
	ret /= 10; /* From 100 nano seconds (10^-7) to 1 microsecond (10^-6) intervals */

	return ret;
}
///string operations
DLLEXPORT int Leviathan::Misc::CutWstring(const wstring& strtocut, const wstring &separator, vector<wstring>& vec){
	// scan the input and gather positions for string copying //
	vector<unique_ptr<Int2>> CopyOperations;
	bool PositionStarted = false;

	for(unsigned int i = 0; i < strtocut.length(); i++){
		if(!PositionStarted){
			PositionStarted = true;
			// add new position index //
			CopyOperations.push_back(unique_ptr<Int2>(new Int2(i, -1)));
		}

		if(strtocut[i] == separator[0]){
			// found occurance
			//test further //
			unsigned int modifier = 0;
			bool WasMatch = false;
			while(strtocut[i+modifier] == separator[modifier]){
				// check can it increase without going out of bounds //
				if((strtocut.length() > i+modifier+1) && (separator.length() > modifier+1)){
					// increase modifier to move forward //
					modifier++;
				} else {
					// check is it a match
					if(modifier+1 == separator.length()){
						// found match! //

						// end old string to just before this position //
						//if(i+1 < strtocut.length()){
						CopyOperations.back()->Y = i; /*-1; not this because we would have to add 1 in the copy phase anyway */
						//} else {
						//	// need a little hack here //
						//	CopyOperations.back()->Val[1] = i-1;
						//}
						PositionStarted = false;
						// skip separator //
						WasMatch = true;
						break;
					}
					break;
				}
			}

			// skip the separator amount of characters, if it was found //
			if(WasMatch)
				// -1 here so that first character of next string won't be missing, because of the loop incrementation //
				i += separator.length()-1;

		}

	}

	if(CopyOperations.size() < 2){
		// would be just one string, for legacy reasons we return nothing //
		vec.clear();
		return 404;
	}

	// make sure final position has end //
	if(CopyOperations.back()->Y < 0)
		CopyOperations.back()->Y = strtocut.length();
	// length-1 is not used here, because it would have to be added in copy phase to the substring length, and we didn't add that earlier... //

	// make space //
	vec.reserve(CopyOperations.size());

	// loop through positions and copy substrings to result vector //
	for(unsigned int i = 0; i < CopyOperations.size(); i++){
		// copy using std::wstring method for speed //
		vec.push_back(strtocut.substr((unsigned int)CopyOperations[i]->X, (unsigned int)(CopyOperations[i]->Y-CopyOperations[i]->X)));
	}

	// cutting succeeded //
	return 0;
}
int Misc::CountOccuranceWstring(const wstring& data,wstring lookfor){
	if(lookfor.length() < 1)
		return 999;
	if(data.length() < lookfor.length())
		return 0;
	////count occurance
	int count = 0;
	for(unsigned int i = 0; i < data.length();i++){
		if(data[i] == lookfor[0]){
			///found occurance
			//test further
			unsigned int modifier = 0;
			while(data[i+modifier] == lookfor[modifier]){
				///check can it incerase
				if((data.length() > i+modifier+1) && (lookfor.length() > modifier+1)){
					///increase modifier
					modifier++;
				} else {
					///check is it a match
					if(modifier+1 == lookfor.length()){
						///match
						count++;
						break;
					}
					break;
				}
			}

		}
	}

	return count;

}
DLLEXPORT wstring Leviathan::Misc::Replace(const wstring& data, const wstring &toreplace, const wstring &replacer){
	wstring out = L"";
	if(toreplace.size() == 0)
		return data;



	// loop through data and copy final characters to out string //
	for(unsigned int i = 0; i < data.size(); i++){
		// check for replaced part //
		if(data[i] == toreplace[0]){
			// check for match //
			bool IsMatch = false;
			for(unsigned int checkind = 0; (checkind < toreplace.size()) && (checkind < data.size()); checkind++){
				if(data[i+checkind] != toreplace[checkind]){
					// didn't match //
					break;
				}
				// check is final iteration //
				if(!((checkind+1 < toreplace.size()) && (checkind+1 < data.size()))){
					// is a match //
					IsMatch = true;
					break;
				}
			}
			if(IsMatch || toreplace.size() == 1){
				// it is a match, copy everything in replacer and add toreplace length to i //
				out += replacer;

				i += toreplace.length()-1;
				continue;
			}
		}
		// non matching character, just copy to result //
		out += data[i];
	}

	return out;
}
void Misc::ReplaceWord(wstring& data, wstring toreplace, wstring replacer){

	if((toreplace.size() == 0) | (data.size() == 0))
		return;
	for(unsigned int i = 0; i < data.size(); i++){
		if(data[i] == toreplace[0]){
			if((i != 0)){
				if(data[i-1] != L' '){
					// part of other word //
					continue;
				}
			}
			// first is a match //
			bool NoMatch = false;
			for(unsigned int a = 1; a < toreplace.size(); a++){
				if(i+a >= data.size())
					return; // no space for match //
				if(data[i+a] != toreplace[a]){
					// no match
					NoMatch = true;
					break;
				}
			}
			if(NoMatch)
				continue;
			if((i+toreplace.size() < data.size())){
				if(data[i+toreplace.size()] != L' '){
					// part of other word //
					continue;
				}
			}
			// match replace //
			//int RemoveInd = 0;
			//for(int a = 0; a < toreplace.size(); a++){
			//	if(i+a < replacer.size()){
			//		// there are characters to replace with //
			//		data[i+a] = toreplace[a];
			//		continue;
			//	}
			//	// no more characters, remove //
			//	data.erase(data.begin()+i+a+RemoveInd);
			//	RemoveInd--;
			//}
			bool CharsEnded = false;
			bool OverWriteEnded = false;

			int RemoveInd = 0;
			unsigned int a = 0;

			bool Working = true;

			while(Working){
				if(a >= replacer.size())
					CharsEnded = true;
				if(a >= toreplace.size())
					OverWriteEnded = true;
				if(CharsEnded && OverWriteEnded){
					Working = false;
					break;
				}
				if(!CharsEnded && !OverWriteEnded){
					// copy from replacer over to data on top of toreplace //
					data[i+a] = replacer[a];

				} else if(CharsEnded && !OverWriteEnded){
					// remove chars //
					data.erase(data.begin()+i+a+RemoveInd);
					RemoveInd--;
				} else if(!CharsEnded && OverWriteEnded){
					// add chars //
					data.insert(data.begin()+i+a, replacer[a]);
					//RemoveInd++;
				}

				a++;
			}
		}
	}
}
bool Misc::WstringContains(const wstring& data, wchar_t check){
	for(unsigned int i = 0; i < data.length(); i++){
		if(data[i] == check)
			return true;
	}
	return false;
}

bool Leviathan::Misc::WstringContainsNumbers(const wstring& data){
	for(unsigned int i = 0; i < data.length(); i++){
		if((int)data[i] >= 48 && (int)data[i] <= 57)
			return true;
	}
	return false;
}

int Misc::WstringGetSecondWord(const wstring& data, wstring& result){
	result = L"";
	int spaces = 0;
	bool Datafound = false;
	for(unsigned int i = 0; i < data.length(); i++){
		if(data[i] == L' '){
			if(Datafound)
				break;
			spaces++;
			continue;
		}
		if(spaces > 0){

			result += data[i];
			Datafound = true;
		}
	}
	return 0;
}

int Misc::WstringGetFirstWord(const wstring& data, wstring& result){
	result = L"";
	for(unsigned int i = 0; i < data.length(); i++){
		if(data[i] == L' '){
			break;
		}

		result += data[i];

	}
	return 0;
}

bool Misc::WstringStartsWith(const wstring& data, const wstring& lookfor){
	if(data.size() < lookfor.size())
		return false;
	for(unsigned int i = 0; i < lookfor.size(); i++){
		if(data[i] != lookfor[i])
			return false;
	}
	return true;
}

wstring Misc::WstringRemoveFirstWords(wstring& data, int amount){
	wstring result = L"";
	int words = 0;
	int spaces = 0;

	bool Datafound = false;
	for(unsigned int i = 0; i < data.length(); i++){
		if(Datafound){
			result += data[i];
			continue;
		}
		if(data[i] == L' '){
			spaces++;
			continue;
		}
		if(spaces > 0){
			words++;
			if(words == amount){
				Datafound = true;
				result += data[i];
			}
			spaces = 0;
		}
		
	}

	return result;
}

wstring Misc::WstringStitchTogether(vector<wstring*> data, wstring separator){
	wstring ret = L"";
	bool first = true;
	// reserve space //
	int totalcharacters = 0;
	for(unsigned int i = 0; i < data.size(); i++){
		totalcharacters += data[i]->length();
	}
	totalcharacters += separator.length()*data.size();

	ret.reserve(totalcharacters);

	for(unsigned int i = 0; i < data.size(); i++){
		if(!first)
			ret += separator;
		ret += *data[i];
		first = false;
	}

	return ret;
}

wstring Misc::WstringStitchTogether(vector<shared_ptr<wstring>> data, wstring separator){
	wstring ret = L"";
	bool first = true;
	// reserve space //
	int totalcharacters = 0;
	for(unsigned int i = 0; i < data.size(); i++){
		totalcharacters += data[i]->length();
	}
	totalcharacters += separator.length()*data.size();

	ret.reserve(totalcharacters);

	for(unsigned int i = 0; i < data.size(); i++){
		if(!first)
			ret += separator;
		ret += *data[i];
		first = false;
	}

	return ret;
}


bool Misc::CompareDataBlockTypeToTHISNameCheck(int datablock, int typenamecheckresult){
	if(typenamecheckresult == 0){
		// int //
		if(datablock == DATABLOCK_TYPE_INT)
			return true;
		return false;
	}
	if(typenamecheckresult == 1){
		// float //
		if(datablock == DATABLOCK_TYPE_FLOAT)
			return true;
		return false;
	}
	if(typenamecheckresult == 3){
		// bool //
		if(datablock == DATABLOCK_TYPE_BOOL)
			return true;
		return false;
	}
	if(typenamecheckresult == 4){
		// wstring //
		if(datablock == DATABLOCK_TYPE_WSTRING)
			return true;
		return false;
	}
	if(typenamecheckresult == 5){
		// void //
		if(datablock == DATABLOCK_TYPE_VOIDPTR)
			return true;
		return false;
	}

	return false;
}
bool Misc::WstringCompareInsensitive(const wstring& data, wstring second){
	if(data.size() != second.size())
		return false;

	for(unsigned int i = 0; i < data.size(); i++){
		if(!(Convert::ToLower(data[i]) == Convert::ToLower(second[i]))){
			return false;
		}
	}
	return true;
}
bool Misc::WstringCompareInsensitiveRefs(const wstring& data, const wstring& second){
	if(data.size() != second.size())
		return false;

	for(unsigned int i = 0; i < data.size(); i++){
		if(!(Convert::ToLower(data[i]) == Convert::ToLower(second[i]))){
			return false;
		}
	}
	return true;
}

wstring Misc::Errstring = L"ERROR";
std::string Leviathan::Misc::Errstrings = "ERROR";

std::wstring Leviathan::Misc::ValidNumberCharacters = L"1234567890-+.";
wstring Misc::GetValidCharacters(){
																								   // extended. starts here //
	return L" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~€‚ƒ„…†‡ˆ‰Š‹ŒŽ‘’“”•–—˜™š›œžŸ ¡¢£¤¥¦§¨©ª«¬­®¯°±²³´µ¶·¸¹º»¼½¾¿ÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏÐÑÒÓÔÕÖ×ØÙÚÛÜÝÞßàáâãäåæçèéêëìíîïðñòóôõö÷øùúûüýþÿ";
	//return L" !\"#$%&'()*+-,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}������������������������������׃�����Ѫ�����������             ~����������
	//return L"abcdefghijklmnopqrstuvwxyz���ABCDEFGHIJKLMNOPQRSTUVWXYZ���"


}
// returns 0 for equal 1 for str is before and -1 for tocompare to be before //
DLLEXPORT int Leviathan::Misc::IsWstringBeforeInAlphabet(const wstring& str, const wstring& tocompare){
	// go through first string and see if numbers are before in alphabet //
	for(unsigned int i = 0; i < str.size(); i++){
		if(!(i < tocompare.size())){
			// shorter first //
			return -1;
		}
		// check is str before in "alphabet" char values //
		if((int)str[i] < (int)tocompare[i]){
			return 1;

		} else if((int)str[i] > (int)tocompare[i]){
			// tocompare is bigger, before in "alphabet"
			return -1;
		}

		// last loop check //
		if(!(i+1 < str.size())){
			// they are equal //
			return 0;
		}
	}
	// undefined? //
	return 0;
}

DLLEXPORT bool Leviathan::Misc::IsCharacterNumber(wchar_t chara){
	// check is it in valid characters //
	return Misc::WstringContains(ValidNumberCharacters, chara);
}

DLLEXPORT void Leviathan::Misc::WstringRemovePreceedingTrailingSpaces(wstring& str){
	Int2 CutPositions(-1,-1);

	// search the right part of the string //
	for(unsigned int i = 0; i < str.size(); i++){
		if(!IsCharacterWhiteSpace(str[i])){
			if(CutPositions[0] == -1){
				// beginning ended //
				CutPositions.X = i;
			} else {
				// set last pos as this //

			}
			continue;
		}
		if(CutPositions[0] == -1){
			// still haven't found a character //
			continue;
		}
		// check is this last character //
		unsigned int a = str.size()-1;
		bool found = false;
		for(a; a > i; a--){
			if(!IsCharacterWhiteSpace(str[a])){
				// there is still valid characters //
				found = true;
				break;
			}
		}
		if(found){
			// skip to the found non-space character //
			i = a-1;
			continue;
		}
		// end found //
		CutPositions.Y = i-1;
		break;
	}

	if(CutPositions.X == -1){
		// nothing in the string //
		str.clear();
		return;
	}
	if(CutPositions.Y == -1){
		if(CutPositions.X == -1){
			// just the first character required //
			CutPositions.Y = CutPositions.X;
		} else {
			// no need to cut from the end //
			CutPositions.Y = str.length()-1;
		}
	}

	// set the wstring as it's sub string //
	str = str.substr((UINT)CutPositions[0], (UINT)(CutPositions[1]-CutPositions[0]+1));

	return;
}

DLLEXPORT  bool Leviathan::Misc::IsCharacterWhiteSpace(const wchar_t chara){
	return (int)chara < 33;
}

DLLEXPORT bool Leviathan::Misc::WstringIsNumeric(const wstring &data){
	for(size_t i = 0; i < data.size(); i++){

		if(data[i] == L'+' || data[i] == L'-' || data[i] == L'.' || (data[i] >= L'0' && data[i] <= L'9')){
			// valid //
			continue;
		}
		// non numeric character //
		return false;
	}
	// not found any non numeric characters //
	return true;
}


