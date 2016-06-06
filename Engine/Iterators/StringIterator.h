#pragma once
// ------------------------------------ //
#include "Define.h"
// ------------------------------------ //
#include "StringDataIterator.h"
#include "IteratorData.h"
#include <functional>
#include <memory>

#if defined(_DEBUG) && (defined(LEVIATHAN_BUILD) || !defined(LEVIATHAN_UE_PLUGIN))
#include "Logger.h"
#include "../Utility/Convert.h"
#include "utf8/checked.h"
#endif

namespace Leviathan{

	enum QUOTETYPE {QUOTETYPE_DOUBLEQUOTES, QUOTETYPE_SINGLEQUOTES, QUOTETYPE_BOTH};
    
	enum DECIMALSEPARATORTYPE {
        DECIMALSEPARATORTYPE_DOT,
        DECIMALSEPARATORTYPE_COMMA,
        DECIMALSEPARATORTYPE_BOTH,
        DECIMALSEPARATORTYPE_NONE
    };

	enum UNNORMALCHARACTER{
        UNNORMALCHARACTER_TYPE_NON_ASCII = 0x1,
		UNNORMALCHARACTER_TYPE_CONTROLCHARACTERS = 0x2,
		UNNORMALCHARACTER_TYPE_WHITESPACE = 0x4,
		UNNORMALCHARACTER_TYPE_LOWCODES = 0x8,
		UNNORMALCHARACTER_TYPE_NON_NAMEVALID = 0x10,
		UNNORMALCHARACTER_TYPE_LINEEND = 0x20
    };

	enum EQUALITYCHARACTER{
        EQUALITYCHARACTER_TYPE_EQUALITY,
        EQUALITYCHARACTER_TYPE_DOUBLEDOTSTYLE,
        EQUALITYCHARACTER_TYPE_ALL};

	enum ITERATORCALLBACK_RETURNTYPE{
        ITERATORCALLBACK_RETURNTYPE_STOP,
        ITERATORCALLBACK_RETURNTYPE_CONTINUE
    };

	//! Special case handling flags for iterator
	enum SPECIAL_ITERATOR{

		SPECIAL_ITERATOR_ONNEWLINE_STOP = 0x4,
		//SPECIAL_ITERATOR_ONNEWLINE_WHITESPACE = 0x8,
		//! Causes comments to be handled as whitespace/delimiting
		SPECIAL_ITERATOR_HANDLECOMMENTS_ASSTRING = 0x10,
	};

	//! Common flag for file handling
#define SPECIAL_ITERATOR_FILEHANDLING SPECIAL_ITERATOR_ONNEWLINE_STOP | SPECIAL_ITERATOR_HANDLECOMMENTS_ASSTRING


	//! Set flags for the iterator, this is changed to this for performance
	enum ITERATORFLAG_SET{

		//! \ is found, next special character will be ignored
		ITERATORFLAG_SET_IGNORE_SPECIAL = 0x1,

		//! The iterator has finished the current operation and will stop for now
		ITERATORFLAG_SET_STOP = 0x2,
		//! Iterator is currently inside a string
		ITERATORFLAG_SET_INSIDE_STRING = 0x4,
		//! Iterator is currently inside double quoted string, "like this"
		ITERATORFLAG_SET_INSIDE_STRING_DOUBLE = 0x8,
		//! Iterator is currently inside single quoted string, 'like this'
		ITERATORFLAG_SET_INSIDE_STRING_SINGLE = 0x10,
		//! Iterator is currently on the closing ' character and the string might end after this character
		ITERATORFLAG_SET_INSIDE_STRING_SINGLE_END = 0x20,

		//! Iterator is currently on the closing " character and the string might end after this character
		ITERATORFLAG_SET_INSIDE_STRING_DOUBLE_END = 0x40,

		//! Set when the next character is no longer affected by \, meaning this is always set when ITERATORFLAG_SET_IGNORE_SPECIAL is set
		ITERATORFLAG_SET_IGNORE_SPECIAL_END = 0x80,

		//! Set when a comment is beginning. The iterator is currently on a / character which is followed by a / or *
		ITERATORFLAG_SET_COMMENT_BEGINNING = 0x100,
		//! Set when a comment is active, the iterator is on either the beginning // or /* or the ending */ or the line end
		ITERATORFLAG_SET_INSIDE_COMMENT = 0x200,
		//! Set when inside a // comment
		ITERATORFLAG_SET_INSIDE_CPPCOMMENT = 0x400,
		//! Set when inside a /* comment
		ITERATORFLAG_SET_INSIDE_CCOMMENT = 0x800,
		//! Set when a c++ style comment will end on the next character
		ITERATORFLAG_SET_CPPCOMMENT_END = 0x1000,
		//! Set when a c style comment will end on the next character
		ITERATORFLAG_SET_CCOMMENT_END = 0x2000
	};


		//0x4000
		//0x8000 // second byte full (int range might stop here(
		//0x10000
		//0x20000
		//0x40000
		//0x80000
		//0x100000
		//0x200000
		//0x400000
		//0x800000 // third byte full
		//0x1000000
		//0x2000000
		//0x4000000
		//0x8000000
		//0x10000000
		//0x20000000
		//0x40000000
		//0x80000000 // fourth byte full (will need QWORD here)
		//0x100000000



	//! Iterator class for getting parts of a string
	class StringIterator{
	public:
		//! \brief Creates a iterator from the iterating object
		//! \param TakesOwnership Set to true when iterator should be deleted by this object
		DLLEXPORT StringIterator(StringDataIterator* iterator, bool TakesOwnership = false);	

		//! \brief Helper constructor for common string type
		DLLEXPORT StringIterator(const std::wstring &text);
		//! \brief Helper constructor for common string type
		DLLEXPORT StringIterator(const std::string &text);
		//! \brief Helper constructor for common string type
		//! \param text Pointer to a string that won't be deleted by this
		DLLEXPORT StringIterator(const std::wstring* text);
		//! \brief Helper constructor for common string type
		//! \param text Pointer to a string that won't be deleted by this
		DLLEXPORT StringIterator(const std::string* text);

        //! \brief Creates an empty iterator
        //!
        //! Use ReInit to fill with data
        DLLEXPORT StringIterator();


		DLLEXPORT virtual ~StringIterator();

		//! \brief Changes the current iterator to the new iterator and goes to the beginning
		DLLEXPORT void ReInit(StringDataIterator* iterator, bool TakesOwnership = false);
		//! \brief Helper function for ReInit for common string type
		DLLEXPORT void ReInit(const std::wstring &text);
		//! \brief Helper function for ReInit for common string type
		DLLEXPORT void ReInit(const std::string &text);
		//! \brief Helper function for ReInit for common string type
		//! \param text Pointer to a string that won't be deleted by this
		DLLEXPORT void ReInit(const std::wstring* text);
		//! \brief Helper function for ReInit for common string type
		//! \param text Pointer to a string that won't be deleted by this
		DLLEXPORT void ReInit(const std::string* text);

		// Iterating functions //

		//! \brief Gets the next string in quotes
		//!
		//! This function will skip until it finds a quote (either " or ' specified by quotes)
        //! and then returns the content inside
		//! \return The string found or NULL
		template<class RStrType>
		DLLEXPORT std::unique_ptr<RStrType> GetStringInQuotes(QUOTETYPE quotes,
            int specialflags = 0)
        {

			// Setup the result object //
			IteratorPositionData data;

			// Iterate with our getting function //
			StartIterating(specialflags, &StringIterator::FindFirstQuotedString, &data, quotes, specialflags);

			// Create the substring from the result //
            std::unique_ptr<RStrType> resultstr;

			// NULL if nothing found //
			if(!data.Positions.Start || !data.Positions.End)
				return nullptr;

			// Return the wanted part //
			return GetSubstringFromIndexes<RStrType>(data.Positions.Start, data.Positions.End);
		}

		//! \brief Gets the next number
		//!
		//! This function will skip until it finds a number and returns the number string
        //! according to the decimal parameter.
		//! If the type is DECIMALSEPARATORTYPE_NONE decimal numbers are only read until the dot
		//! \return The string found or NULL
		template<class RStrType>
		DLLEXPORT std::unique_ptr<RStrType> GetNextNumber(DECIMALSEPARATORTYPE decimal,
            int specialflags = 0)
        {

			// Setup the result object //
			IteratorNumberFindData data;

			// iterate over the string getting the proper part //
			// Iterate with our getting function //
			StartIterating(specialflags, &StringIterator::FindNextNumber, &data, decimal, specialflags);

			// Check for nothing found //
			if(!data.Positions.Start){
				return nullptr;
			}

			// create substring of the wanted part //


			// Make sure end is fine //
			if(!data.Positions.End)
				data.Positions.End = GetLastValidCharIndex();

			// Return the wanted part //
			return GetSubstringFromIndexes<RStrType>(data.Positions.Start, data.Positions.End);
		}

		//! \brief Gets the next sequence of characters according to stopcaseflags
		//! \param stopcaseflags Specifies until what type of characters this string is read.
		//! Should be created by using UNNORMALCHARACTER as bit flags inside the argument int
		//! \return The string found or NULL
		template<class RStrType>
		DLLEXPORT std::unique_ptr<RStrType> GetNextCharacterSequence(int stopcaseflags,
            int specialflags = 0)
        {

			// Setup the result object //
			IteratorPositionData data;

			// Iterate with our getting function //
			StartIterating(specialflags, &StringIterator::FindNextNormalCharacterString, &data, stopcaseflags, 
                specialflags);

			// check for nothing found //
			if(!data.Positions.Start && !data.Positions.Start){

				return NULL;
			}

			// Make sure end is fine //
			if(!data.Positions.End)
				data.Positions.End = GetLastValidCharIndex();
			

			// Return the wanted part //
			return GetSubstringFromIndexes<RStrType>(data.Positions.Start, data.Positions.End);
		}

		//! \brief Gets the string that is before the equality assignment
		//!
		//! This function will read until either : or = is encountered specified by stopcase
		//! \return The string found or NULL
		template<class RStrType>
		DLLEXPORT std::unique_ptr<RStrType> GetUntilEqualityAssignment(EQUALITYCHARACTER stopcase,
            int specialflags = 0)
        {
			
			// Setup the result object //
			IteratorAssignmentData data;

			// Iterate with our getting function //
			StartIterating(specialflags, &StringIterator::FindUntilEquality, &data, stopcase, specialflags);

			// Check for validity //
			if(!data.Positions.Start || data.Positions.Start == data.Positions.End || data.SeparatorFound == false){
				// nothing found //
				return nullptr;
			}

			if(!data.Positions.End){
				// Set to start, this only happens if there is just one character //
				data.Positions.End = data.Positions.Start;
			}

			// Return the wanted part //
			return GetSubstringFromIndexes<RStrType>(data.Positions.Start, data.Positions.End);
		}


		//! \brief Gets all characters until the end
		//! \note This does not advance the iterator so this object can still be used after this
		//! \return The string found or NULL if the read position is invalid
		template<class RStrType>
		DLLEXPORT std::unique_ptr<RStrType> GetUntilEnd(){

			// Just return from here to the last character //
			return GetSubstringFromIndexes<RStrType>(GetPosition(), GetLastValidCharIndex());
		}

		//! \brief Gets all characters until a line end
		//!
		//! This function will read until a new line character and end after it
		//! \return The string found or NULL
		template<class RStrType>
		DLLEXPORT std::unique_ptr<RStrType> GetUntilLineEnd(){

			// Setup the result object //
			IteratorFindUntilData data;

			// Iterate with our getting function //
			StartIterating(0, &StringIterator::FindUntilNewLine, &data);

			// Check for validity //
			if(!data.Positions.Start){
				// Nothing found //
				return nullptr;
			}

			if(!data.Positions.End){
				// Set to end of string //
				data.Positions.End = GetLastValidCharIndex();
			}

			// Return the wanted part //
			return GetSubstringFromIndexes<RStrType>(data.Positions.Start, data.Positions.End);
		}



		//! \brief Gets characters until a character or nothing if the specified character is not found
		//!
		//! This function will read until charactertolookfor and return the string without charactertolookfor, or if not found nothing
		//! \param charactertolookfor The code point to look for
		//! \return The string found or NULL
		//! \see GetUntilNextCharacterOrAll
		template<class RStrType>
		DLLEXPORT std::unique_ptr<RStrType> GetUntilNextCharacterOrNothing(int charactertolookfor,
            int specialflags = 0)
        {

			auto data = GetPositionsUntilACharacter(charactertolookfor, specialflags);

			// Check was the end found //
			if(!data.FoundEnd || !data.Positions.Start){
				// not found the ending character //
				return nullptr;
			}

			// Return the wanted part //
			return GetSubstringFromIndexes<RStrType>(data.Positions.Start, data.Positions.End);
		}

		//! \brief Gets characters until a character or all remaining characters
		//!
		//! This function will read until charactertolookfor and return the string without
        //! charactertolookfor, or if not found until the end
		//! \param charactertolookfor The code point to look for
		//! \return The string found or NULL if there are no valid characters left
		//! \see GetUntilNextCharacterOrAll GetUntilEnd
		template<class RStrType>
		DLLEXPORT std::unique_ptr<RStrType> GetUntilNextCharacterOrAll(int charactertolookfor,
            int specialflags = 0)
        {

			auto data = GetPositionsUntilACharacter(charactertolookfor, specialflags);

			if(!data.Positions.Start|| !data.Positions.End){
				// return empty string //
				return nullptr;
			}

			// Return all if not found //
			if(!data.FoundEnd){

				return GetSubstringFromIndexes<RStrType>(data.Positions.Start,
                    GetLastValidCharIndex());
			}

			// Return the wanted part //
			return GetSubstringFromIndexes<RStrType>(data.Positions.Start, data.Positions.End);
		}

		//! \brief Gets all characters until a sequence is matched
		//! \return The string found or NULL
		//! \bug Finding until an UTF-8 sequence doesn't work, the findstr parameter should be
        //! a StringDataIterator for it to work
		template<class RStrType>
		DLLEXPORT std::unique_ptr<RStrType> GetUntilCharacterSequence(const RStrType &findstr,
            int specialflags = 0)
        {

			// Setup the result object //
			IteratorUntilSequenceData<RStrType> data(findstr);

			// Iterate with our getting function //
			StartIterating(specialflags, &StringIterator::FindUntilSequence<RStrType>, &data, specialflags);

			// Check for validity //
			if(!data.Positions.Start){
				// Nothing found //
				return nullptr;
			}

			// This only happens when the string ends with a partial match //
			// Example: look for "this", string is like this: my super nice th
			if(!data.Positions.End){
				// Set to end of string //
				data.Positions.End = GetLastValidCharIndex();
			}

			// Return the wanted part //
			return GetSubstringFromIndexes<RStrType>(data.Positions.Start, data.Positions.End);
		}

        //! \brief Gets characters inside brackets
		//!
		//! This function will skip until it finds a a left bracket '['
        //! and then returns the content inside keeping track of the number of '[' and ']'
        //! characters encountered and returns once the top level brackets close
        //! \note Empty brackets "[]" are considered invalid and return NULL
		//! \return The string found or NULL
		template<class RStrType>
		DLLEXPORT std::unique_ptr<RStrType> GetStringInBracketsRecursive(int specialflags = 0){

			// Setup the result object //
			IteratorNestingLevelData data;

			// Iterate with our getting function //
			StartIterating(specialflags, &StringIterator::FindInMatchingParentheses, &data, '[', ']', specialflags);

			// Create the substring from the result //
            std::unique_ptr<RStrType> resultstr;

			// NULL if nothing found //
			if(!data.Positions.Start || !data.Positions.End)
				return nullptr;

			// Return the wanted part //
			return GetSubstringFromIndexes<RStrType>(data.Positions.Start, data.Positions.End);
		}

		//! \brief Skips until characters that are not whitespace are found
		//! \see SkipCharacters
		DLLEXPORT void inline SkipWhiteSpace(int specialflags = 0){

			SkipCharacters(32, UNNORMALCHARACTER_TYPE_LOWCODES, specialflags);
		}

		//! \brief Skips until chartoskip doesn't match the current character
		//! \param chartoskip The code point to skip
		//! \param additionalflag Flag composing of UNNORMALCHARACTER_TYPE which defines additional things to skip
		//! \see SkipWhiteSpace
		DLLEXPORT void SkipCharacters(int chartoskip, int additionalflag = 0,
            int specialflags = 0)
        {

            IteratorCharacterData stufftoskip(chartoskip);

			// Iterate over the string skipping until hit something that doesn't need to be
            // skipped
			StartIterating(specialflags, &StringIterator::SkipSomething, stufftoskip, additionalflag, specialflags);
		}

		// Utility functions //

#if defined(_DEBUG) && (defined(LEVIATHAN_BUILD) || !defined(LEVIATHAN_UE_PLUGIN))
		//! \brief When set to true prints lots of debug output
		DLLEXPORT void SetDebugMode(const bool &mode);
#endif

		//! \brief Returns the current reading position
		//! \note This might be somewhat expensive operation based on the underlying StringDataIterator
        //! class (mostly expensive for UTF8 strings)
		DLLEXPORT size_t GetPosition();

		//! \brief Returns the current line the processing is happening
		DLLEXPORT size_t GetCurrentLine();

		//! \brief Gets the character in the position current + forward
		DLLEXPORT int GetCharacter(size_t forward = 0);

		//! \brief Gets the previous character
		DLLEXPORT int GetPreviousCharacter();

		//! \brief Returns the last valid index on the iterator
		DLLEXPORT size_t GetLastValidCharIndex() const;

		//! \brief Skips the current character and moves to the next
		//! \return True when there is a valid character or false if the end
        //! has already been reached
		DLLEXPORT bool MoveToNext();

		//! \brief Returns true when the read position is valid
		DLLEXPORT bool IsOutOfBounds();

		//! \brief Returns substring from the wanted indexes
		template<class STRSType>
		DLLEXPORT std::unique_ptr<STRSType> GetSubstringFromIndexes(size_t firstcharacter,
            size_t lastcharacter) const
        {
			// Don't want to do anything if no string //
			if(!DataIterator)
				return nullptr;

			// Return a substring from our data source //
            std::unique_ptr<STRSType> returnval(new STRSType());

			if(DataIterator->ReturnSubString(firstcharacter, lastcharacter, *returnval)){

				return returnval;
			}

			// It failed for some reason //
			return nullptr;
		}


		//! \brief Gets the position of the current character and the specified character
		DLLEXPORT IteratorFindUntilData GetPositionsUntilACharacter(int character,
            int specialflags = 0)
        {
			// Setup the result object //
			IteratorFindUntilData data;

			// Iterate with our getting function //
			StartIterating(specialflags, &StringIterator::FindUntilSpecificCharacter, &data, character, specialflags);

#if defined(_DEBUG) && (defined(LEVIATHAN_BUILD) || !defined(LEVIATHAN_UE_PLUGIN))
			if(DebugMode){
				Logger::Get()->Write("Iterator: find GetPositionsUntilACharacter, positions: "+
                    Convert::ToString(data.Positions) + ", found: "+Convert::ToString(data.FoundEnd));
			}
#endif // _DEBUG

			return data;
		}

	private:

		StringIterator(const StringIterator &other) = delete;


		inline ITERATORCALLBACK_RETURNTYPE HandleSpecialCharacters();
		inline ITERATORCALLBACK_RETURNTYPE CheckActiveFlags();

        template <typename T, typename R, typename ...Args>
        R proxycall(T & obj, R(T::*mf)(Args...), Args &&... args) {
            return (obj.*mf)(std::forward<Args>(args)...);
        }

		//! \brief Loops over the string using functorun to handle continuing
		//! \param specialflagcopy Depending on set flags this can cause the iterator to treat
        //! line ends as if they were end of input
        template <typename... Params, typename... Args>
        void StartIterating(int specialflagcopy, 
            ITERATORCALLBACK_RETURNTYPE(StringIterator::*mf)(Params...), Args&&... args)
        {
        #if defined(_DEBUG) && (defined(LEVIATHAN_BUILD) || !defined(LEVIATHAN_UE_PLUGIN))
            if (DebugMode) {
                Logger::Get()->Write("Iterator: begin ----------------------");
            }
        #endif // _DEBUG
            // We want to skip multiple checks on same character so we skip checks on first character when starting except the beginning of the string //
            bool IsStartUpLoop = GetPosition() == 0 ? true : false;

            bool firstiter = true;
            if (IsStartUpLoop)
                firstiter = false;

            for (; DataIterator->IsPositionValid(); DataIterator->MoveToNextCharacter()) {

                // The GetCharacter call will cache the result but there might be iterators that don't
                // want to get the current character
            #if defined(_DEBUG) && (defined(LEVIATHAN_BUILD) || !defined(LEVIATHAN_UE_PLUGIN))
                int chara = GetCharacter();

                if (DebugMode) {
                    // Convert to UTF8 //

                    string datathing = "Iterator: iterating: " + Convert::ToString(GetPosition()) + " (";

                    // Encode the character //
                    utf8::utf32to8(&chara, (&chara) + 1, back_inserter(datathing));

                    datathing += ")";

                    Logger::Get()->Write(datathing);
                }
            #endif // _DEBUG

                // First iteration call is the same as the last so skip it //
                if (!firstiter) {

                    if (CheckActiveFlags() == ITERATORCALLBACK_RETURNTYPE_STOP)
                        break;

                    // check current character //
                    if (HandleSpecialCharacters() == ITERATORCALLBACK_RETURNTYPE_STOP)
                        break;
                }

                firstiter = false;

                // Check for special cases //

                // valid character/valid iteration call callback //
                ITERATORCALLBACK_RETURNTYPE retval = (this->*mf)(/*std::forward<Args>*/(args)...);
                if (retval == ITERATORCALLBACK_RETURNTYPE_STOP) {

#if defined(_DEBUG) && (defined(LEVIATHAN_BUILD) || !defined(LEVIATHAN_UE_PLUGIN))
                    if (DebugMode) {
                        Logger::Get()->Write("Iterator: stop ----------------------");
                    }
#endif // _DEBUG
                    break;
                }

                // Character changes after this //
                CurrentStored = false;
            }
        }


		// ------------------------------------ //
		//! Set when we should delete DataIterator
		bool HandlesDelete;

#if defined(_DEBUG) && (defined(LEVIATHAN_BUILD) || !defined(LEVIATHAN_UE_PLUGIN))
		//! Controls debug output printing
		bool DebugMode = false;
#endif
		
		//! Wraps the underlying string
		StringDataIterator* DataIterator;

		//! Currently active flags
		//! Bit field of ITERATORFLAG_SET enum values
		int CurrentFlags;

		//! Stored for performance
		int CurrentCharacter;
		//! Dirty flag for CurrentCharacter
		bool CurrentStored;

	protected:

		// Iteration functions //

		DLLEXPORT inline ITERATORCALLBACK_RETURNTYPE FindFirstQuotedString(IteratorPositionData* data,
            QUOTETYPE quotes, int specialflags);
        
		DLLEXPORT inline ITERATORCALLBACK_RETURNTYPE FindNextNormalCharacterString(
            IteratorPositionData* data, int stopflags, int specialflags);
        
		DLLEXPORT inline ITERATORCALLBACK_RETURNTYPE FindNextNumber(IteratorNumberFindData* data,
            DECIMALSEPARATORTYPE decimal, int specialflags);
        
		DLLEXPORT inline ITERATORCALLBACK_RETURNTYPE FindUntilEquality(IteratorAssignmentData* data,
            EQUALITYCHARACTER equality, int specialflags);
        
		DLLEXPORT inline ITERATORCALLBACK_RETURNTYPE SkipSomething(IteratorCharacterData &data,
            const int additionalskip, const int specialflags);
        
		DLLEXPORT inline ITERATORCALLBACK_RETURNTYPE FindUntilSpecificCharacter(
            IteratorFindUntilData* data, int character, int specialflags);
        
		DLLEXPORT inline ITERATORCALLBACK_RETURNTYPE FindUntilNewLine(IteratorFindUntilData* data);

        DLLEXPORT inline ITERATORCALLBACK_RETURNTYPE FindInMatchingParentheses(
            IteratorNestingLevelData* data, int left, int right, int specialflags);

		template<class AcceptStr>
		DLLEXPORT inline ITERATORCALLBACK_RETURNTYPE FindUntilSequence(
            IteratorUntilSequenceData<AcceptStr>* data, int specialflags)
        {
			// First check if this is a line end //
			int curcharacter = GetCharacter();

			if(curcharacter == '\n' && specialflags & SPECIAL_ITERATOR_ONNEWLINE_STOP){

				// Set the end to one before this, if found any //
				if(!data->Positions.Start){

					data->Positions.End = GetPosition() - 1;
				}

				return ITERATORCALLBACK_RETURNTYPE_STOP;
			}

			// We may not be inside strings nor comments for checking //
			if(!(CurrentFlags & ITERATORFLAG_SET_INSIDE_STRING) &&
                (!(CurrentFlags & ITERATORFLAG_SET_INSIDE_COMMENT) || 
				!(specialflags & SPECIAL_ITERATOR_HANDLECOMMENTS_ASSTRING)))
			{
				// Check do we match the current position //
				if(curcharacter == data->StringToMatch[data->CurMatchedIndex]){

					// Found a matching character //

					// Move to next match position and don't yet verify if this is a valid
                    // character
					++data->CurMatchedIndex;

					if(data->CurMatchedIndex >= data->StringToMatch.size()){

						// End found //
						return ITERATORCALLBACK_RETURNTYPE_STOP;
					}

					return ITERATORCALLBACK_RETURNTYPE_CONTINUE;

				}

			}

			// Go back to beginning of matching //
			data->CurMatchedIndex = 0;

			// All is fine //
			if(!data->Positions.Start){

				data->Positions.Start = GetPosition();
				data->Positions.End = data->Positions.Start;
			} else {

				// This might be poisonous to performance, but this gets the job done //
				data->Positions.End = GetPosition();
			}

			return ITERATORCALLBACK_RETURNTYPE_CONTINUE;
		}


	};

}

