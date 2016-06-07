#pragma once
// ------------------------------------ //
#include <string>
#include <vector>
#include "../Utility/Convert.h"
#include <memory>

namespace Leviathan{

	//! \brief Helper class that provides string constants in multiple types
	template<class StringWanted, typename ElementType>
	class StringConstants{
	public:

		// Public variables //
		static const ElementType DotCharacter;
		static const ElementType UniversalPathSeparator;
		static const ElementType WindowsPathSeparator;
		static const ElementType SpaceCharacter;

		static const ElementType FirstNumber;
		static const ElementType LastNumber;
		static const ElementType Dash;
		static const ElementType PlusSymbol;

		static const StringWanted WindowsLineSeparator;
		static const StringWanted UniversalLineSeparator;


	private:
		StringConstants();
		~StringConstants();
	};

	// Define most common StringConstants types //
	typedef StringConstants<std::wstring, wchar_t> WstringConstants;
	typedef StringConstants<std::string, char> NarrowStringConstants;

	// ------------------ StringConstants definitions for new types that comply with char
    // ------------------ 
	template<class StringWanted, typename ElementType> const ElementType
    StringConstants<StringWanted, ElementType>::DotCharacter = (ElementType)(int)'.';
    
	template<class StringWanted, typename ElementType> const ElementType
    StringConstants<StringWanted, ElementType>::UniversalPathSeparator = (ElementType)(int)'/';
    
	template<class StringWanted, typename ElementType> const ElementType
    StringConstants<StringWanted, ElementType>::WindowsPathSeparator = (ElementType)(int)'\\';
    
	template<class StringWanted, typename ElementType> const ElementType
    StringConstants<StringWanted, ElementType>::SpaceCharacter = (ElementType)(int)' ';
	
	template<class StringWanted, typename ElementType> const ElementType
    StringConstants<StringWanted, ElementType>::FirstNumber = (ElementType)(int)'0';
    
	template<class StringWanted, typename ElementType> const ElementType
    StringConstants<StringWanted, ElementType>::LastNumber = (ElementType)(int)'9';
    
	template<class StringWanted, typename ElementType> const ElementType
    StringConstants<StringWanted, ElementType>::Dash = (ElementType)(int)'-';
    
	template<class StringWanted, typename ElementType> const ElementType
    StringConstants<StringWanted, ElementType>::PlusSymbol = (ElementType)(int)'+';

	//! \brief Singleton class that has string processing functions
	//!
	//! Most functions work with any type of string, but it is recommended to only pass string
    //! or wstring to avoid headaches.
    //! \todo Get rid of ElementType and just use ints for everything and hope that it is large
    //! enough
	class StringOperations{
	public:
		template<typename CharType>
		DLLEXPORT static bool IsCharacterWhitespace(CharType character){
			if((int)character <= 32)
				return true;
			return false;
		}

		// ------------------ Path related operations ------------------ //
		template<class StringTypeN, typename CharType>
		DLLEXPORT static const StringTypeN RemoveExtension(const StringTypeN &filepath, bool delpath = true){

			size_t startcopy = 0;
			size_t endcopy;

			size_t lastdot = filepath.find_last_of(StringConstants<StringTypeN, CharType>::DotCharacter);

			if(lastdot == StringTypeN::npos){
				// no dot //
				endcopy = filepath.size()-1;
			} else {
				endcopy = lastdot-1;
			}

			// Potentially erase from beginning //
			if(delpath){
				// Find last path character //
				size_t lastpath = 0;

				for(size_t i = 0; i < filepath.size(); i++){
					if(filepath[i] == StringConstants<StringTypeN,
                        CharType>::UniversalPathSeparator || filepath[i] ==
                        StringConstants<StringTypeN, CharType>::WindowsPathSeparator)
					{
						// Currently last found path //
						lastpath = i;
					}
				}

				if(lastpath != 0){
					// Set start //
					startcopy = lastpath+1;
				}
			}

			// Return empty if no data is valid //
			if(startcopy > endcopy || startcopy >= filepath.size() || endcopy >= filepath.size())
				return StringTypeN();

			// return the wanted part //
			return filepath.substr(startcopy, endcopy-startcopy+1);
		}

		template<class StringTypeN, typename CharType>
		DLLEXPORT static const StringTypeN GetExtension(const StringTypeN &filepath){
			size_t startcopy = 0;
			size_t endcopy = filepath.size()-1;

			size_t lastdot = filepath.find_last_of(StringConstants<StringTypeN,
                CharType>::DotCharacter);

			if(lastdot == StringTypeN::npos){
				// no dot //
				return StringTypeN();
			}

			startcopy = lastdot+1;

			// Return empty if no data is valid //
			if(startcopy > endcopy || startcopy >= filepath.size() || endcopy >= filepath.size())
				return StringTypeN();

			// Return the extension //
			return filepath.substr(startcopy, endcopy-startcopy+1);
		}

		template<class StringTypeN, typename CharType>
		DLLEXPORT static const StringTypeN ChangeExtension(const StringTypeN& filepath,
            const StringTypeN &newext)
        {
			size_t startcopy = 0;
			size_t endcopy = filepath.size()-1;

			size_t lastdot = filepath.find_last_of(StringConstants<StringTypeN,
                CharType>::DotCharacter);

			if(lastdot != StringTypeN::npos){
				// dot found //
				endcopy = lastdot;

			} else {
				// No dot, so just append it //
				return filepath+newext;
			}

			// Return empty if no data is valid //
			if(startcopy > endcopy || startcopy >= filepath.size() || endcopy >= filepath.size())
				return StringTypeN();

			// Return the extension //
			return filepath.substr(startcopy, endcopy-startcopy+1)+newext;
		}

		template<class StringTypeN, typename CharType>
		DLLEXPORT static const StringTypeN RemovePath(const StringTypeN &filepath){
			size_t startcopy = 0;
			size_t endcopy = filepath.size()-1;

			// Find last path character //
			size_t lastpath = 0;

			for(size_t i = 0; i < filepath.size(); i++){
				if(filepath[i] == StringConstants<StringTypeN, CharType>::UniversalPathSeparator
                    || filepath[i] == StringConstants<StringTypeN, CharType>::WindowsPathSeparator)
				{
					// Currently last found path //
					lastpath = i;
				}
			}

			if(lastpath != 0){
				// Set start //
				startcopy = lastpath+1;
			}

			// Return empty if no data is valid //
			if(startcopy > endcopy || startcopy >= filepath.size() || endcopy >= filepath.size())
				return StringTypeN();
			

			// return the wanted part //
			return filepath.substr(startcopy, endcopy-startcopy+1);
		}

		//! \brief Returns the path part of a path+filename
		template<class StringTypeN, typename CharType>
		DLLEXPORT static const StringTypeN GetPath(const StringTypeN &filepath){
			size_t startcopy = 0;
			size_t endcopy = filepath.size()-1;

			// Find last path character //
			size_t lastpath;
            bool found = false;

			for(size_t i = 0; i < filepath.size(); i++){
				if(filepath[i] == StringConstants<StringTypeN, CharType>::UniversalPathSeparator || filepath[i] == 
					StringConstants<StringTypeN, CharType>::WindowsPathSeparator)
				{
					// Currently last found path //
					lastpath = i;
                    found = true;
				}
			}

			if(!found){
				// Set start //
				return StringTypeN();
			}

			// Set up copy //
			endcopy = lastpath;


			// Return empty if no data is valid //
			if(startcopy > endcopy || startcopy >= filepath.size() || endcopy >= filepath.size())
				return StringTypeN();


			// return the wanted part //
			return filepath.substr(startcopy, endcopy-startcopy+1);
		}



		//! \brief Changes all line separators to Windows line separators
		template<class StringTypeN, typename CharType>
		DLLEXPORT static const StringTypeN ChangeLineEndsToWindows(const StringTypeN &input){

			StringTypeN results;

			// Try to find path strings and replace them //
			size_t copystart = 0;
			size_t copyend = 0;

			for(size_t i = 0; i < input.size(); i++){
				if(input[i] == StringConstants<StringTypeN, CharType>::UniversalLineSeparator[0]
                    && i > 0 && input[i-1] != StringConstants<StringTypeN,
                    CharType>::WindowsLineSeparator[0])
				{
					// Found a line separator //
					// Copy the current thing //
					if(copyend >= copystart && copystart-copyend > 1)
						results += input.substr(copystart, copyend-copystart+1);

					results += StringConstants<StringTypeN, CharType>::WindowsLineSeparator;

					copystart = i+1 < input.size() ? i+1: i;
					copyend = copystart;

					i += 1;

					continue;
				}
				
				// Change the end copy //
				copyend = i;
			}

			if(copyend >= copystart && copystart-copyend > 1)
				results += input.substr(copystart, copyend-copystart+1);


			return results;
		}

		//! \brief Changes all line separators to universal line separators
		template<class StringTypeN, typename CharType>
		DLLEXPORT static const StringTypeN ChangeLineEndsToUniversal(const StringTypeN &input){

			StringTypeN results;

			// Try to find path strings and replace them //
			size_t copystart = 0;
			size_t copyend = 0;

			for(size_t i = 0; i < input.size(); i++){
				if(input[i] == StringConstants<StringTypeN, CharType>::WindowsLineSeparator[0] &&
                    i+1 < input.size() && input[i+1] == StringConstants<StringTypeN,
                    CharType>::WindowsLineSeparator[1])
				{
					// Found a line separator //
					// Copy the current thing //
					if(copyend >= copystart && copystart-copyend > 1)
						results += input.substr(copystart, copyend-copystart+1);

					results += StringConstants<StringTypeN, CharType>::UniversalLineSeparator;

					copystart = i+2 < input.size() ? i+2: i;
					copyend = copystart;

					i += 2;

					continue;
				}

				// Change the end copy //
				copyend = i;
			}

			if(copyend >= copystart && copystart-copyend > 1)
				results += input.substr(copystart, copyend-copystart+1);


			return results;
		}


		// ------------------ General string operations ------------------ //
		template<class StringTypeN>
		DLLEXPORT static bool CutString(const StringTypeN &strtocut, const StringTypeN &separator,
            std::vector<StringTypeN>& vec)
        {
			// scan the input and gather positions for string copying //
            std::vector<StartEndIndex> CopyOperations;
			bool PositionStarted = false;

			for(size_t i = 0; i < strtocut.length(); i++){
				if(!PositionStarted){
					PositionStarted = true;
					// add new position index //
					CopyOperations.push_back(StartEndIndex(i));
				}

				if(strtocut[i] == separator[0]){
					// Found a possible match //
					// test further //
					size_t modifier = 0;
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
								CopyOperations.back().End = i; /*-1;
                                                               not this because we would have to
                                                               add 1 in the copy phase anyway */

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
						// -1 here so that first character of next string won't be missing,
                        // because of the loop incrementation
						i += separator.length()-1;
				}
			}

			if(CopyOperations.size() < 2){
				// would be just one string, for legacy
                // (actually we don't want caller to think it got cut) reasons we return nothing //
				return false;
			}

			// make sure final position has end //
			if(!CopyOperations.back().End)
				CopyOperations.back().End = strtocut.length();
			// length-1 is not used here, because it would have to be added in copy phase to the
            // substring length, and we didn't add that earlier...

			// make space //
			vec.reserve(CopyOperations.size());

			// loop through positions and copy substrings to result vector //
			for(size_t i = 0; i < CopyOperations.size(); i++){
				// copy using std::wstring method for speed //
				vec.push_back(strtocut.substr(static_cast<size_t>(CopyOperations[i].Start),
                        static_cast<size_t>(CopyOperations[i].End) - static_cast<size_t>(CopyOperations[i].Start)));
			}

			// cutting succeeded //
			return true;
		}

		template<class StringTypeN>
		DLLEXPORT static int CountOccuranceInString(const StringTypeN &data,
            const StringTypeN &lookfor)
        {

			int count = 0;

			for(size_t i = 0; i < data.length(); i++){
				if(data[i] == lookfor[0]){
					// Found a possible match //
					// test further //
					size_t modifier = 0;
					bool WasMatch = false;
					while(data[i+modifier] == lookfor[modifier]){
						// check can it increase without going out of bounds //
						if((data.length() > i+modifier+1) && (lookfor.length() > modifier+1)){
							// increase modifier to move forward //
							modifier++;
						} else {
							// check is it a match
							if(modifier+1 == lookfor.length()){
								// found match! //
								count++;
								WasMatch = true;
								break;
							}
							break;
						}
					}
					// skip the separator amount of characters, if it was found //
					if(WasMatch)
						// -1 here so that first character of next string won't be missing,
                        // because of the loop incrementation
						i += lookfor.length()-1;
				}
			}
            
			return count;
		}

		template<class StringTypeN>
		DLLEXPORT static StringTypeN Replace(const StringTypeN &data,
            const StringTypeN &toreplace, const StringTypeN &replacer)
        {
			// We construct an output string from the wanted bits //
			StringTypeN out;

			if(toreplace.size() < 1){
				// Don't replace anything //
				return data;
			}

            PotentiallySetIndex copystart;
            PotentiallySetIndex copyend;

			// loop through data and copy final characters to out string //
			for(size_t i = 0; i < data.size(); i++){
				// check for replaced part //
				if(data[i] == toreplace[0]){
					// check for match //
					bool IsMatch = false;
					for(size_t checkind = 0; (checkind < toreplace.size()) &&
                            (checkind < data.size()); checkind++)
                    {
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
						// First add proper characters //
						if(copystart && copyend)
							out += data.substr(copystart, 
                                static_cast<size_t>(copyend) - static_cast<size_t>(copystart) + 1);

                        copystart.ValueSet = false;
                        copyend.ValueSet = false;

						// it is a match, copy everything in replacer and add toreplace length to i //
						out += replacer;

						i += toreplace.length()-1;
						continue;
					}
				}
                
				// non matching character mark as to copy //
				if(!copystart){
					copystart = i;
				} else {
					copyend = i;
				}
			}

			// Copy rest to out //
			if(copystart && copyend)
				out += data.substr(copystart, static_cast<size_t>(copyend) - static_cast<size_t>(copystart) + 1);

			// Return finished string //
			return out;
		}

		template<class StringTypeN, typename CharType>
		DLLEXPORT static StringTypeN RemoveFirstWords(const StringTypeN &data, int amount){

			size_t firstpos = 0;
			// Find the copy start position //
			int spaces = 0;
			int words = 0;

			for(size_t i = 0; i < data.length(); i++){
				if(data[i] == StringConstants<StringTypeN, CharType>::SpaceCharacter){
					spaces++;
					continue;
				}
				if(spaces > 0){
					words++;
					if(words == amount){
						// This is the spot we want to start from //
						firstpos = i;
						break;
					}
					spaces = 0;
				}
			}

			if(firstpos == 0){
				// Didn't remove anything? //
				return data;
			}

			// Generate sub string from start to end //
			return data.substr(firstpos, data.size()-firstpos);
		}

		template<class StringTypeN>
		DLLEXPORT static StringTypeN StitchTogether(const std::vector<StringTypeN*> &data,
            const StringTypeN &separator)
        {
			StringTypeN ret;
			bool first = true;
			// reserve space //
			int totalcharacters = 0;

			// This might be faster than not reserving space //
			for(size_t i = 0; i < data.size(); i++){
				totalcharacters += data[i]->length();
			}
            
			totalcharacters += separator.length()*data.size();
			
			// By reserving space we don't have to allocate more memory during copying which might
            // be faster
			ret.reserve(totalcharacters);

			for(size_t i = 0; i < data.size(); i++){
				if(!first)
					ret += separator;
				ret += *data[i];
				first = false;
			}

			return ret;
		}

		template<class StringTypeN>
		DLLEXPORT static StringTypeN StitchTogether(
            const std::vector<std::shared_ptr<StringTypeN>> &data, const StringTypeN &separator)
        {
			StringTypeN ret;
			bool first = true;
			// reserve space //
			int totalcharacters = 0;

			// This might be faster than not reserving space //
			for(size_t i = 0; i < data.size(); i++){
				totalcharacters += data[i]->length();
			}
			totalcharacters += separator.length()*data.size();

			// By reserving space we don't have to allocate more memory during copying which might
            // be faster
			ret.reserve(totalcharacters);

			for(size_t i = 0; i < data.size(); i++){
				if(!first)
					ret += separator;
				ret += *data[i].get();
				first = false;
			}

			return ret;
		}

		template<class StringTypeN>
		DLLEXPORT static void RemovePreceedingTrailingSpaces(StringTypeN &str){
			StartEndIndex CutPositions;

			// search the right part of the string //
			for(size_t i = 0; i < str.size(); i++){
				if(!IsCharacterWhitespace(str[i])){
					if(!CutPositions.Start){
						// beginning ended //
						CutPositions.Start = i;
					} else {
						// set last pos as this //

					}
					continue;
				}
				if(!CutPositions.Start){
					// still haven't found a character //
					continue;
				}
				// check is this last character //
				size_t a = str.size()-1;
				bool found = false;
				for(; a > i; a--){
					if(!IsCharacterWhitespace(str[a])){
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
				CutPositions.End = i-1;
				break;
			}

			if(!CutPositions.Start){
				// nothing in the string //
				str.clear();
				return;
			}
			if(!CutPositions.End){
				if(!CutPositions.Start){
					// just the first character required //
					CutPositions.End = CutPositions.Start;
				} else {
					// no need to cut from the end //
					CutPositions.End = str.length()-1;
				}
			}

			// set the wstring as it's sub string //
			str = str.substr(static_cast<size_t>(CutPositions.Start), 
                static_cast<size_t>(CutPositions.End) - static_cast<size_t>(CutPositions.Start) + 1);
		}

		template<class StringTypeN>
		DLLEXPORT static bool CompareInsensitive(const StringTypeN &data,
            const StringTypeN &second)
        {
			if(data.size() != second.size())
				return false;

			for(unsigned int i = 0; i < data.size(); i++){
                if(data[i] != second[i]){

                    // Check are they different case //
                    if(97 <= data[i] && data[i] <= 122){

                        if(data[i]-32 != second[i]){

                            return false;
                        }
                    } else if(97 <= second[i] && second[i] <= 122){

                        if(second[i]-32 != data[i]){

                            return false;
                        }
                    } else {
                        
                        return false;
                    }
                }
			}
            
			return true;
		}

		template<class StringTypeN>
		DLLEXPORT static bool StringStartsWith(const StringTypeN &data,
            const StringTypeN &tomatch)
        {
			size_t foundstop = data.find(tomatch);
			return foundstop != StringTypeN::npos && foundstop == 0;
		}
        
		template<class StringTypeN, typename CharType>
		DLLEXPORT static bool IsStringNumeric(const StringTypeN &data){
			for(size_t i = 0; i < data.size(); i++){
				if((data[i] < StringConstants<StringTypeN, CharType>::FirstNumber ||
                        data[i] > StringConstants<StringTypeN, CharType>::LastNumber) &&
					data[i] != StringConstants<StringTypeN, CharType>::Dash &&
                    data[i] != StringConstants<StringTypeN, CharType>::DotCharacter &&
					data[i] != StringConstants<StringTypeN, CharType>::PlusSymbol)
				{
					return false;
				}
			}
			return true;
		}

        //! \todo Make this work with any unicode characters
        template<class StringTypeN>
        DLLEXPORT static StringTypeN ToUpperCase(const StringTypeN &data){

            StringTypeN result;
            result.reserve(data.size());
            
            for(size_t i = 0; i < data.size(); i++){

                // Not actually unicode decoding...
                auto const codepoint = data[i];

                if(97 <= codepoint && codepoint <= 122){

                    result.push_back(codepoint-32);
                    
                } else {

                    result.push_back(codepoint);
                }
            }

            return result;
        }

        //! \returns True if a character is a line terminating character
        DLLEXPORT static bool IsLineTerminator(int32_t codepoint) {

            if (codepoint == '\r' || codepoint == '\n' ||
                // Unicode newlines //
                codepoint == 0x0085 || codepoint == 0x2028 || codepoint == 0x2029 ||
                codepoint == 0x000B || codepoint == 0x000C)
            {
                return true;
            }

            return false;
        }

        //! \returns True if two characters are a line terminating sequence
        DLLEXPORT static bool IsLineTerminator(int32_t codepoint1, int32_t codepoint2) {

            if (codepoint1 == '\r' && codepoint2 == '\n')
            {
                return true;
            }

            return false;
        }

		// ------------------ Named non-template versions ------------------ //
		DLLEXPORT FORCE_INLINE static const std::wstring GetExtensionWstring(
            const std::wstring &filepath)
        {
			return GetExtension<std::wstring, wchar_t>(filepath);
		}
        
		DLLEXPORT FORCE_INLINE static const std::string GetExtensionString(
            const std::string &filepath)
        {
			return GetExtension<std::string, char>(filepath);
		}

		DLLEXPORT FORCE_INLINE static const std::wstring GetPathWstring(
            const std::wstring &filepath)
        {
			return GetPath<std::wstring, wchar_t>(filepath);
		}
        
		DLLEXPORT FORCE_INLINE static const std::string GetPathString(const std::string &filepath){
            
			return GetPath<std::string, char>(filepath);
		}

		DLLEXPORT FORCE_INLINE static const std::wstring RemoveExtensionWstring(
            const std::wstring &filepath, bool delpath = true)
        {
			return RemoveExtension<std::wstring, wchar_t>(filepath, delpath);
		}
        
		DLLEXPORT FORCE_INLINE static const std::string RemoveExtensionString(
            const std::string &filepath, bool delpath = true)
        {
			return RemoveExtension<std::string, char>(filepath, delpath);
		}

		DLLEXPORT FORCE_INLINE static const std::wstring ChangeExtensionWstring(
            const std::wstring &filepath, const std::wstring &newext)
        {
			return ChangeExtension<std::wstring, wchar_t>(filepath, newext);
		}
        
		DLLEXPORT FORCE_INLINE static const std::string ChangeExtensionString(
            const std::string &filepath, const std::string &newext)
        {
			return ChangeExtension<std::string, char>(filepath, newext);
		}

		DLLEXPORT FORCE_INLINE static const std::wstring RemovePathWstring(
            const std::wstring &filepath)
        {
			return RemovePath<std::wstring, wchar_t>(filepath);
		}
        
		DLLEXPORT FORCE_INLINE static const std::string RemovePathString(
            const std::string &filepath)
        {
			return RemovePath<std::string, char>(filepath);
		}

		DLLEXPORT FORCE_INLINE static const std::wstring ChangeLineEndsToUniversalWstring(
            const std::wstring &input)
        {
				return ChangeLineEndsToUniversal<std::wstring, wchar_t>(input);
		}
        
		DLLEXPORT FORCE_INLINE static const std::string ChangeLineEndsToUniversalString(
            const std::string &input)
        {
			return ChangeLineEndsToUniversal<std::string, char>(input);
		}

		DLLEXPORT FORCE_INLINE static const std::wstring ChangeLineEndsToWindowsWstring(
            const std::wstring &input)
        {
			return ChangeLineEndsToWindows<std::wstring, wchar_t>(input);
		}
        
		DLLEXPORT FORCE_INLINE static const std::string ChangeLineEndsToWindowsString(
            const std::string &input)
        {
			return ChangeLineEndsToWindows<std::string, char>(input);
		}

	private:
		StringOperations();
		~StringOperations();
	};



}


