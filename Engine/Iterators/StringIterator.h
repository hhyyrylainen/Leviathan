#pragma once
// ------------------------------------ //
#include "Define.h"
// ------------------------------------ //
#include "../Common/StringOperations.h"
#include "IteratorData.h"
#include "StringDataIterator.h"

#include <functional>
#include <memory>


// Defining debug macro //
#if defined(_DEBUG) && (defined(LEVIATHAN_BUILD) || !defined(LEVIATHAN_UE_PLUGIN))
#define ALLOW_DEBUG
#define ITERATOR_ALLOW_DEBUG
#include "../Utility/Convert.h"
#include "Logger.h"
#include "utf8/checked.h"
#endif

#ifdef ALLOW_DEBUG
#define ITR_FUNCDEBUG(x)                                                   \
    {                                                                      \
        if(DebugMode) {                                                    \
            Logger::Get()->Write("Iterator: procfunc: " + std::string(x)); \
        }                                                                  \
    }

#define ITR_COREDEBUG(x)                                         \
    {                                                            \
        if(DebugMode) {                                          \
            Logger::Get()->Write("Iterator: " + std::string(x)); \
        }                                                        \
    }
#else
#define ITR_FUNCDEBUG(x) \
    {}
#define ITR_COREDEBUG(x) \
    {}
#endif // _DEBUG


namespace Leviathan {

enum QUOTETYPE { QUOTETYPE_DOUBLEQUOTES, QUOTETYPE_SINGLEQUOTES, QUOTETYPE_BOTH };

enum DECIMALSEPARATORTYPE {
    DECIMALSEPARATORTYPE_DOT,
    DECIMALSEPARATORTYPE_COMMA,
    DECIMALSEPARATORTYPE_BOTH,
    DECIMALSEPARATORTYPE_NONE
};

enum UNNORMALCHARACTER {
    UNNORMALCHARACTER_TYPE_NON_ASCII = 0x1,
    UNNORMALCHARACTER_TYPE_CONTROLCHARACTERS = 0x2,
    UNNORMALCHARACTER_TYPE_WHITESPACE = 0x4,
    UNNORMALCHARACTER_TYPE_LOWCODES = 0x8,
    UNNORMALCHARACTER_TYPE_NON_NAMEVALID = 0x10,
    UNNORMALCHARACTER_TYPE_LINEEND = 0x20
};

enum EQUALITYCHARACTER {
    EQUALITYCHARACTER_TYPE_EQUALITY,
    EQUALITYCHARACTER_TYPE_DOUBLEDOTSTYLE,
    EQUALITYCHARACTER_TYPE_ALL
};

enum ITERATORCALLBACK_RETURNTYPE {
    ITERATORCALLBACK_RETURNTYPE_STOP,
    ITERATORCALLBACK_RETURNTYPE_CONTINUE
};

//! Special case handling flags for iterator
enum SPECIAL_ITERATOR {

    SPECIAL_ITERATOR_ONNEWLINE_STOP = 0x4,
    // SPECIAL_ITERATOR_ONNEWLINE_WHITESPACE = 0x8,
    //! Causes comments to be handled as whitespace/delimiting
    SPECIAL_ITERATOR_HANDLECOMMENTS_ASSTRING = 0x10,
};

//! Common flag for file handling
#define SPECIAL_ITERATOR_FILEHANDLING \
    SPECIAL_ITERATOR_ONNEWLINE_STOP | SPECIAL_ITERATOR_HANDLECOMMENTS_ASSTRING


//! Set flags for the iterator, this is changed to this for performance
enum ITERATORFLAG_SET {

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
    //! Iterator is currently on the closing '
    //! character and the string might end after this character
    ITERATORFLAG_SET_INSIDE_STRING_SINGLE_END = 0x20,

    //! Iterator is currently on the closing " character
    //! and the string might end after this character
    ITERATORFLAG_SET_INSIDE_STRING_DOUBLE_END = 0x40,

    //! Set when the next character is no longer affected by \,
    //! meaning this is always set when ITERATORFLAG_SET_IGNORE_SPECIAL is set
    ITERATORFLAG_SET_IGNORE_SPECIAL_END = 0x80,

    //! Set when a comment is beginning. The iterator is currently on a / character
    //! which is followed by a / or *
    ITERATORFLAG_SET_COMMENT_BEGINNING = 0x100,
    //! Set when a comment is active, the iterator is on either the beginning // or /*
    //! or the ending */ or the line end
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




//! Iterator class for getting parts of a string
class StringIterator {
public:
    //! \brief Creates a iterator from the iterating object
    DLLEXPORT StringIterator(std::unique_ptr<StringDataIterator>&& iterator);

    //! \brief Helper constructor for common string type
    DLLEXPORT StringIterator(const std::wstring& text);
    //! \brief Helper constructor for common string type
    DLLEXPORT StringIterator(const std::string& text);
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
    DLLEXPORT void ReInit(std::unique_ptr<StringDataIterator>&& iterator);
    //! \brief Helper function for ReInit for common string type
    DLLEXPORT void ReInit(const std::wstring& text);
    //! \brief Helper function for ReInit for common string type
    DLLEXPORT void ReInit(const std::string& text);
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
    std::unique_ptr<RStrType> GetStringInQuotes(QUOTETYPE quotes, int specialflags = 0)
    {
        // Setup the result object //
        IteratorPositionData data;

        // Iterate with our getting function //
        StartIterating(
            specialflags, &StringIterator::FindFirstQuotedString, &data, quotes, specialflags);

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
    std::unique_ptr<RStrType> GetNextNumber(DECIMALSEPARATORTYPE decimal, int specialflags = 0)
    {
        // Setup the result object //
        IteratorNumberFindData data;

        // iterate over the string getting the proper part //
        // Iterate with our getting function //
        StartIterating(
            specialflags, &StringIterator::FindNextNumber, &data, decimal, specialflags);

        // Check for nothing found //
        if(!data.Positions.Start) {
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
    std::unique_ptr<RStrType> GetNextCharacterSequence(int stopcaseflags, int specialflags = 0)
    {
        // Setup the result object //
        IteratorPositionData data;

        // Iterate with our getting function //
        StartIterating(specialflags, &StringIterator::FindNextNormalCharacterString, &data,
            stopcaseflags, specialflags);

        // check for nothing found //
        if(!data.Positions.Start && !data.Positions.Start) {

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
    std::unique_ptr<RStrType> GetUntilEqualityAssignment(
        EQUALITYCHARACTER stopcase, int specialflags = 0)
    {
        // Setup the result object //
        IteratorAssignmentData data;

        // Iterate with our getting function //
        StartIterating(
            specialflags, &StringIterator::FindUntilEquality, &data, stopcase, specialflags);

        // Check for validity //
        if(!data.Positions.Start || data.SeparatorFound == false) {
            // nothing found //
            return nullptr;
        }

        if(!data.Positions.End) {
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
    std::unique_ptr<RStrType> GetUntilEnd()
    {

        // Just return from here to the last character //
        return GetSubstringFromIndexes<RStrType>(GetPosition(), GetLastValidCharIndex());
    }

    //! \brief Gets all characters until a line end
    //!
    //! This function will read until a new line character and end after it
    //! \return The string found or NULL
    template<class RStrType>
    std::unique_ptr<RStrType> GetUntilLineEnd()
    {

        // Setup the result object //
        IteratorFindUntilData data;

        // Iterate with our getting function //
        StartIterating(0, &StringIterator::FindUntilNewLine, &data);

        // Check for validity //
        if(!data.Positions.Start) {
            // Nothing found //
            return nullptr;
        }

        if(!data.Positions.End) {
            // Set to end of string //
            data.Positions.End = GetLastValidCharIndex();
        }

        // Return the wanted part //
        return GetSubstringFromIndexes<RStrType>(data.Positions.Start, data.Positions.End);
    }



    //! \brief Gets characters until a character or nothing
    //! if the specified character is not found
    //!
    //! This function will read until charactertolookfor and return the string without
    //! charactertolookfor, or if not found nothing
    //! \param charactertolookfor The code point to look for
    //! \return The string found or NULL
    //! \see GetUntilNextCharacterOrAll
    template<class RStrType>
    std::unique_ptr<RStrType> GetUntilNextCharacterOrNothing(
        int charactertolookfor, int specialflags = 0)
    {

        auto data = GetPositionsUntilACharacter(charactertolookfor, specialflags);

        // Check was the end found //
        if(!data.FoundEnd || !data.Positions.Start) {
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
    std::unique_ptr<RStrType> GetUntilNextCharacterOrAll(
        int charactertolookfor, int specialflags = 0)
    {
        auto data = GetPositionsUntilACharacter(charactertolookfor, specialflags);

        if(!data.Positions.Start || !data.Positions.End) {
            // return empty string //
            return nullptr;
        }

        // Return all if not found //
        if(!data.FoundEnd &&
            (!data.NewLineBreak || !(specialflags & SPECIAL_ITERATOR_ONNEWLINE_STOP))) {
            return GetSubstringFromIndexes<RStrType>(
                data.Positions.Start, GetLastValidCharIndex());
        }

        // Return the wanted part //
        return GetSubstringFromIndexes<RStrType>(data.Positions.Start, data.Positions.End);
    }

    //! \brief Gets all characters until a sequence is matched
    //! \return The string found or NULL
    //! \bug Finding until an UTF-8 sequence doesn't work, the findstr parameter should be
    //! a StringDataIterator for it to work
    template<class RStrType>
    std::unique_ptr<RStrType> GetUntilCharacterSequence(
        const RStrType& findstr, int specialflags = 0)
    {
        // Setup the result object //
        IteratorUntilSequenceData<RStrType> data(findstr);

        // Iterate with our getting function //
        StartIterating(
            specialflags, &StringIterator::FindUntilSequence<RStrType>, &data, specialflags);

        // Check for validity //
        if(!data.Positions.Start) {
            // Nothing found //
            return nullptr;
        }

        // This only happens when the string ends with a partial match //
        // Example: look for "this", string is like this: my super nice th
        if(!data.Positions.End) {
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
    std::unique_ptr<RStrType> GetStringInBracketsRecursive(int specialflags = 0)
    {

        // Setup the result object //
        IteratorNestingLevelData data;

        // Iterate with our getting function //
        StartIterating(specialflags, &StringIterator::FindInMatchingParentheses, &data, '[',
            ']', specialflags);

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
    void inline SkipWhiteSpace(int specialflags = 0)
    {

        SkipCharacters(32, UNNORMALCHARACTER_TYPE_LOWCODES, specialflags);
    }

    //! \brief Skips until chartoskip doesn't match the current character
    //! \param chartoskip The code point to skip
    //! \param additionalflag Flag composing of UNNORMALCHARACTER_TYPE
    //! which defines additional things to skip
    //! \see SkipWhiteSpace
    void SkipCharacters(int chartoskip, int additionalflag = 0, int specialflags = 0)
    {
        IteratorCharacterData stufftoskip(chartoskip);

        // Iterate over the string skipping until hit something that doesn't need to be
        // skipped
        StartIterating(specialflags, &StringIterator::SkipSomething, stufftoskip,
            additionalflag, specialflags);
    }

    // Utility functions //

#ifdef ITERATOR_ALLOW_DEBUG
    //! \brief When set to true prints lots of debug output
    void SetDebugMode(bool mode)
    {
        DebugMode = mode;
    }
#endif

    //! \brief Returns the current reading position
    //! \note This might be somewhat expensive operation based on the
    //! underlying StringDataIterator
    //! class (mostly expensive for UTF8 strings)
    inline size_t GetPosition()
    {

        return DataIterator->CurrentIteratorPosition();
    }

    //! \brief Returns the current line the processing is happening
    inline size_t GetCurrentLine()
    {

        return DataIterator->GetCurrentLineNumber();
    }

    //! \brief Gets the character in the position current + forward
    inline int GetCharacter(size_t forward = 0)
    {

        // Special case for current character //
        if(!forward) {

            if(!CurrentStored) {

                if(!DataIterator->GetNextCharCode(CurrentCharacter, 0)) {

                    // Invalid position //
                    return -1;
                }

                CurrentStored = true;

                ITR_COREDEBUG(
                    "Current char: (" + Convert::CodePointToUtf8(CurrentCharacter) + ")");
            }

            return CurrentCharacter;
        }

        // Get the character from our iterator and store it to a temporary value
        // and then return it
        int tmpval = -1;
        DataIterator->GetNextCharCode(tmpval, forward);

        ITR_COREDEBUG("Peek forward char: (" + Convert::CodePointToUtf8(tmpval) + ")");

        return tmpval;
    }

    //! \brief Returns true if current character is a new line
    inline bool IsAtNewLine()
    {

        // Ignore new lines if '\' is put before them
        if(CurrentFlags & ITERATORFLAG_SET_IGNORE_SPECIAL)
            return false;

        return StringOperations::IsLineTerminator(GetCharacter());
    }

    //! \brief Gets the previous character
    DLLEXPORT int GetPreviousCharacter();

    //! \brief Returns the last valid index on the iterator
    inline size_t GetLastValidCharIndex() const
    {

        return DataIterator->GetLastValidIteratorPosition();
    }

    //! \brief Skips the current character and moves to the next
    //! \return True when there is a valid character or false if the end
    //! has already been reached
    inline bool MoveToNext()
    {

        DataIterator->MoveToNextCharacter();
        bool valid = DataIterator->IsPositionValid();
        // It's important to reset this //
        CurrentStored = false;

        // We need to handle the flags on this position if we aren't on the first character //
        if(valid && DataIterator->CurrentIteratorPosition() != 0) {

            ITR_COREDEBUG("Move to next");

            CheckActiveFlags();

            // check current character //
            HandleSpecialCharacters();
        }

        return valid;
    }

    //! \brief Skips characters until the line number changes
    DLLEXPORT void SkipLineEnd();

    //! \brief Returns true when the read position is valid
    inline bool IsOutOfBounds()
    {

        return !DataIterator->IsPositionValid();
    }

    // Flag checking methods for outside callers //
    inline bool IsInsideString()
    {

        return CurrentFlags & ITERATORFLAG_SET_INSIDE_STRING;
    }

    inline bool IsInsideComment()
    {

        return CurrentFlags & ITERATORFLAG_SET_INSIDE_CCOMMENT;
    }

    //! \brief Returns substring from the wanted indexes
    template<class STRSType>
    std::unique_ptr<STRSType> GetSubstringFromIndexes(
        size_t firstcharacter, size_t lastcharacter) const
    {
        // Don't want to do anything if no string //
        if(!DataIterator)
            return nullptr;

        // Return a substring from our data source //
        std::unique_ptr<STRSType> returnval(new STRSType());

        if(DataIterator->ReturnSubString(firstcharacter, lastcharacter, *returnval)) {

            return returnval;
        }

        // It failed for some reason //
        return nullptr;
    }


    //! \brief Gets the position of the current character and the specified character
    IteratorFindUntilData GetPositionsUntilACharacter(int character, int specialflags = 0)
    {
        // Setup the result object //
        IteratorFindUntilData data;

        // Iterate with our getting function //
        StartIterating(specialflags, &StringIterator::FindUntilSpecificCharacter, &data,
            character, specialflags);

#ifdef ITERATOR_ALLOW_DEBUG
        if(DebugMode) {
            Logger::Get()->Write("Iterator: find GetPositionsUntilACharacter, positions: " +
                                 Convert::ToString(data.Positions) +
                                 ", found: " + Convert::ToString(data.FoundEnd));
        }
#endif // _DEBUG

        return data;
    }

private:
    StringIterator(const StringIterator& other) = delete;

    //! Checks if current character causes the flags to change
    inline ITERATORCALLBACK_RETURNTYPE HandleSpecialCharacters()
    {

        // check should this special character be ignored //
        if(CurrentFlags & ITERATORFLAG_SET_IGNORE_SPECIAL)
            return ITERATORCALLBACK_RETURNTYPE_CONTINUE;

        // check for special characters //
        int character = GetCharacter();

        switch(character) {
        case '\\': {
            if(!(CurrentFlags & ITERATORFLAG_SET_INSIDE_COMMENT)) {
                // ignore next special character //
                CurrentFlags |= ITERATORFLAG_SET_IGNORE_SPECIAL;

#ifdef ALLOW_DEBUG
                if(DebugMode) {
                    Logger::Get()->Write("Iterator: setting: ITERATORFLAG_SET_IGNORE_SPECIAL");
                }
#endif // _DEBUG
            }
        } break;
        case '"': {
            // Strings cannot be inside comments //
            if(!(CurrentFlags & ITERATORFLAG_SET_INSIDE_COMMENT)) {
                // a string //
                if(!(CurrentFlags & ITERATORFLAG_SET_INSIDE_STRING_DOUBLE)) {
#ifdef ALLOW_DEBUG
                    if(DebugMode) {
                        Logger::Get()->Write(
                            "Iterator: setting: ITERATORFLAG_SET_INSIDE_STRING_DOUBLE");
                    }

                    if(DebugMode && !(CurrentFlags & ITERATORFLAG_SET_INSIDE_STRING)) {
                        Logger::Get()->Write(
                            "Iterator: setting: ITERATORFLAG_SET_INSIDE_STRING");
                    }
#endif // _DEBUG
       // set //
                    CurrentFlags |= ITERATORFLAG_SET_INSIDE_STRING_DOUBLE;

                    // set as inside string //
                    CurrentFlags |= ITERATORFLAG_SET_INSIDE_STRING;

                } else {
#ifdef ALLOW_DEBUG
                    if(DebugMode) {
                        Logger::Get()->Write("Iterator: set flag end: "
                                             "ITERATORFLAG_SET_INSIDE_STRING_DOUBLE");
                    }
#endif // _DEBUG
       // set ending flag //
                    CurrentFlags |= ITERATORFLAG_SET_INSIDE_STRING_DOUBLE_END;
                }
            }
        } break;
        case '\'': {
            // Strings cannot be inside comments //
            if(!(CurrentFlags & ITERATORFLAG_SET_INSIDE_COMMENT)) {
                // a string //
                if(!(CurrentFlags & ITERATORFLAG_SET_INSIDE_STRING_SINGLE)) {
#ifdef ALLOW_DEBUG
                    if(DebugMode) {
                        Logger::Get()->Write(
                            "Iterator: setting: ITERATORFLAG_SET_INSIDE_STRING_SINGLE");
                    }

                    if(DebugMode && !(CurrentFlags & ITERATORFLAG_SET_INSIDE_STRING)) {
                        Logger::Get()->Write(
                            "Iterator: setting: ITERATORFLAG_SET_INSIDE_STRING");
                    }
#endif // _DEBUG
       // set //
                    CurrentFlags |= ITERATORFLAG_SET_INSIDE_STRING_SINGLE;

                    // set as inside string //
                    CurrentFlags |= ITERATORFLAG_SET_INSIDE_STRING;

                } else {
#ifdef ALLOW_DEBUG
                    if(DebugMode) {
                        Logger::Get()->Write("Iterator: set flag end: "
                                             "ITERATORFLAG_SET_INSIDE_STRING_SINGLE");
                    }
#endif // _DEBUG

                    CurrentFlags |= ITERATORFLAG_SET_INSIDE_STRING_SINGLE_END;
                }
            }
        } break;
        case '/': {
            // Comments cannot be inside strings //
            if(!(CurrentFlags & ITERATORFLAG_SET_INSIDE_STRING)) {
                // There might be a comment beginning //
                int nextchar = GetCharacter(1);

                if(nextchar == '/') {
                    // C++-style comment starts //
                    if(!(CurrentFlags & ITERATORFLAG_SET_INSIDE_CPPCOMMENT)) {
#ifdef ALLOW_DEBUG
                        if(DebugMode) {
                            Logger::Get()->Write(
                                "Iterator: setting: ITERATORFLAG_SET_INSIDE_CPPCOMMENT");
                        }

                        if(DebugMode && !(CurrentFlags & ITERATORFLAG_SET_INSIDE_COMMENT)) {
                            Logger::Get()->Write(
                                "Iterator: setting: ITERATORFLAG_SET_INSIDE_COMMENT");
                        }
#endif // _DEBUG
                        CurrentFlags |= ITERATORFLAG_SET_INSIDE_CPPCOMMENT;
                        CurrentFlags |= ITERATORFLAG_SET_INSIDE_COMMENT;
                    }


                } else if(nextchar == '*') {
                    // C-style comment starts //
                    if(!(CurrentFlags & ITERATORFLAG_SET_INSIDE_CCOMMENT)) {
#ifdef ALLOW_DEBUG
                        if(DebugMode) {
                            Logger::Get()->Write(
                                "Iterator: setting: ITERATORFLAG_SET_INSIDE_CCOMMENT");
                        }

                        if(DebugMode && !(CurrentFlags & ITERATORFLAG_SET_INSIDE_COMMENT)) {
                            Logger::Get()->Write(
                                "Iterator: setting: ITERATORFLAG_SET_INSIDE_COMMENT");
                        }
#endif // _DEBUG
                        CurrentFlags |= ITERATORFLAG_SET_INSIDE_CCOMMENT;
                        CurrentFlags |= ITERATORFLAG_SET_INSIDE_COMMENT;
                    }

                } else if(CurrentFlags & ITERATORFLAG_SET_INSIDE_CCOMMENT) {
                    // C-style comment might end //

                    int previouschar = GetPreviousCharacter();

                    if(previouschar == '*') {

                        // Set as ending //
                        CurrentFlags |= ITERATORFLAG_SET_CCOMMENT_END;
#ifdef ALLOW_DEBUG
                        if(DebugMode) {
                            Logger::Get()->Write(
                                "Iterator: set flag end: ITERATORFLAG_SET_CCOMMENT_END");
                        }
#endif // _DEBUG
                    }
                }
            }

        } break;
        }

        if(IsAtNewLine()) {
            // A C++-style comment might end //
            if(CurrentFlags & ITERATORFLAG_SET_INSIDE_CPPCOMMENT) {
                // Set as ending //
                CurrentFlags |= ITERATORFLAG_SET_CPPCOMMENT_END;
#ifdef ALLOW_DEBUG
                if(DebugMode) {
                    Logger::Get()->Write(
                        "Iterator: set flag end: ITERATORFLAG_SET_CPPCOMMENT_END");
                }
#endif // _DEBUG
            }
        }

        return ITERATORCALLBACK_RETURNTYPE_CONTINUE;
    }

    //! Makes flags that are due to end end
    inline ITERATORCALLBACK_RETURNTYPE CheckActiveFlags()
    {
        if(CurrentFlags & ITERATORFLAG_SET_STOP)
            return ITERATORCALLBACK_RETURNTYPE_STOP;

        // Reset 1 character long flags //
        if(CurrentFlags & ITERATORFLAG_SET_IGNORE_SPECIAL) {
#ifdef ALLOW_DEBUG
            if(DebugMode) {
                Logger::Get()->Write("Iterator: flag: STRINGITERATOR_IGNORE_SPECIAL");
            }
#endif // _DEBUG

            // check should end now //
            if(CurrentFlags & ITERATORFLAG_SET_IGNORE_SPECIAL_END) {
#ifdef ALLOW_DEBUG
                if(DebugMode) {
                    Logger::Get()->Write(
                        "Iterator: flag ended: STRINGITERATOR_IGNORE_SPECIAL");
                }
#endif // _DEBUG
       // unset both //
                CurrentFlags &= ~ITERATORFLAG_SET_IGNORE_SPECIAL_END;
                CurrentFlags &= ~ITERATORFLAG_SET_IGNORE_SPECIAL;

            } else {
#ifdef ALLOW_DEBUG
                if(DebugMode) {
                    Logger::Get()->Write("Iterator: flag ends next character: "
                                         "ITERATORFLAG_SET_IGNORE_SPECIAL");
                }
#endif // _DEBUG
       // set to end next character //
                CurrentFlags |= ITERATORFLAG_SET_IGNORE_SPECIAL_END;
            }
        }

        // reset end flags before we process this cycle further //
        if(CurrentFlags & ITERATORFLAG_SET_INSIDE_STRING_DOUBLE_END) {
#ifdef ALLOW_DEBUG
            if(DebugMode) {
                Logger::Get()->Write(
                    "Iterator: flag ends: ITERATORFLAG_SET_INSIDE_STRING_DOUBLE");
            }
#endif // _DEBUG
       // unset flag //
            CurrentFlags &= ~ITERATORFLAG_SET_INSIDE_STRING_DOUBLE;
            CurrentFlags &= ~ITERATORFLAG_SET_INSIDE_STRING_DOUBLE_END;
        }

        if(CurrentFlags & ITERATORFLAG_SET_INSIDE_STRING_SINGLE_END) {
#ifdef ALLOW_DEBUG
            if(DebugMode) {
                Logger::Get()->Write(
                    "Iterator: flag ends: ITERATORFLAG_SET_INSIDE_STRING_SINGLE");
            }
#endif // _DEBUG
       // unset flag //
            CurrentFlags &= ~ITERATORFLAG_SET_INSIDE_STRING_SINGLE;
            CurrentFlags &= ~ITERATORFLAG_SET_INSIDE_STRING_SINGLE_END;
        }

        // Check can we unset the whole string flag //
        if(CurrentFlags & ITERATORFLAG_SET_INSIDE_STRING) {
            if(!(CurrentFlags & ITERATORFLAG_SET_INSIDE_STRING_DOUBLE) &&
                !(CurrentFlags & ITERATORFLAG_SET_INSIDE_STRING_SINGLE)) {
#ifdef ALLOW_DEBUG
                if(DebugMode) {
                    Logger::Get()->Write(
                        "Iterator: flag ends: ITERATORFLAG_SET_INSIDE_STRING");
                }
#endif // _DEBUG
       // can unset this //
                CurrentFlags &= ~ITERATORFLAG_SET_INSIDE_STRING;
            }
        }

        // Unsetting comment flags //
        if(CurrentFlags & ITERATORFLAG_SET_CPPCOMMENT_END) {
#ifdef ALLOW_DEBUG
            if(DebugMode) {
                Logger::Get()->Write("Iterator: flag ends: ITERATORFLAG_SET_CPPCOMMENT_END");
            }
#endif // _DEBUG
       // unset flag //
            CurrentFlags &= ~ITERATORFLAG_SET_CPPCOMMENT_END;
            CurrentFlags &= ~ITERATORFLAG_SET_INSIDE_CPPCOMMENT;
        }

        // C-style flag //
        if(CurrentFlags & ITERATORFLAG_SET_CCOMMENT_END) {
#ifdef ALLOW_DEBUG
            if(DebugMode) {
                Logger::Get()->Write("Iterator: flag ends: ITERATORFLAG_SET_CCOMMENT_END");
            }
#endif // _DEBUG
       // unset flag //
            CurrentFlags &= ~ITERATORFLAG_SET_CCOMMENT_END;
            CurrentFlags &= ~ITERATORFLAG_SET_INSIDE_CCOMMENT;
        }

        // Check can we unset the whole comment flag //
        if(CurrentFlags & ITERATORFLAG_SET_INSIDE_COMMENT) {
            if(!(CurrentFlags & ITERATORFLAG_SET_INSIDE_CPPCOMMENT) &&
                !(CurrentFlags & ITERATORFLAG_SET_INSIDE_CCOMMENT)) {
#ifdef ALLOW_DEBUG
                if(DebugMode) {
                    Logger::Get()->Write(
                        "Iterator: flag ends: ITERATORFLAG_SET_INSIDE_COMMENT");
                }
#endif // _DEBUG
       // can unset this //
                CurrentFlags &= ~ITERATORFLAG_SET_INSIDE_COMMENT;
            }
        }


        return ITERATORCALLBACK_RETURNTYPE_CONTINUE;
    }

    template<typename T, typename R, typename... Args>
    R proxycall(T& obj, R (T::*mf)(Args...), Args&&... args)
    {
        return (obj.*mf)(std::forward<Args>(args)...);
    }

    //! \brief Loops over the string using functorun to handle continuing
    //! \param specialflagcopy Depending on set flags this can cause the iterator to treat
    //! line ends as if they were end of input
    template<typename... Params, typename... Args>
    inline void StartIterating(int specialflagcopy,
        ITERATORCALLBACK_RETURNTYPE (StringIterator::*mf)(Params...), Args&&... args)
    {
#ifdef ITERATOR_ALLOW_DEBUG
        if(DebugMode) {
            Logger::Get()->Write("Iterator: begin ----------------------");
        }
#endif // ITERATOR_ALLOW_DEBUG
       // We want to skip multiple checks on same character
       // so we skip checks on first character when starting except
       // the beginning of the string
        bool IsStartUpLoop = GetPosition() == 0 ? true : false;

        bool firstiter = true;
        if(IsStartUpLoop)
            firstiter = false;

        for(; DataIterator->IsPositionValid(); DataIterator->MoveToNextCharacter()) {

            // The GetCharacter call will cache the result
            // but there might be iterators that don't want to get the current character
#ifdef ITERATOR_ALLOW_DEBUG

            if(DebugMode) {
                int chara = GetCharacter();

                std::string datathing =
                    "Iterator: iterating: " + Convert::ToString(GetPosition()) + " (";

                // Encode the character //
                datathing += Convert::CodePointToUtf8(chara) + ")";

                Logger::Get()->Write(datathing);
            }

#endif // ITERATOR_ALLOW_DEBUG

            // First iteration call is the same as the last so skip it //
            if(!firstiter) {

                if(CheckActiveFlags() == ITERATORCALLBACK_RETURNTYPE_STOP)
                    break;

                // check current character //
                if(HandleSpecialCharacters() == ITERATORCALLBACK_RETURNTYPE_STOP)
                    break;
            }

            firstiter = false;

            // Check for special cases //

            // valid character/valid iteration call callback //
            ITERATORCALLBACK_RETURNTYPE retval = (this->*mf)(std::forward<Args>(args)...);
            if(retval == ITERATORCALLBACK_RETURNTYPE_STOP) {

#ifdef ITERATOR_ALLOW_DEBUG
                if(DebugMode) {
                    Logger::Get()->Write("Iterator: stop ----------------------");
                }
#endif // ITERATOR_ALLOW_DEBUG
                break;
            }

            // Character changes after this //
            CurrentStored = false;
        }
    }


    // ------------------------------------ //
    //! Set when we should delete DataIterator
    bool HandlesDelete = false;

#ifdef ITERATOR_ALLOW_DEBUG
    //! Controls debug output printing
    bool DebugMode = false;
#endif

    //! Wraps the underlying string
    StringDataIterator* DataIterator = nullptr;

    //! Currently active flags
    //! Bit field of ITERATORFLAG_SET enum values
    int CurrentFlags = 0;

    //! Stored for performance
    int CurrentCharacter = -1;

    //! Dirty flag for CurrentCharacter
    bool CurrentStored = false;

protected:
    // Iteration functions //

    inline ITERATORCALLBACK_RETURNTYPE FindFirstQuotedString(
        IteratorPositionData* data, QUOTETYPE quotes, int specialflags)
    {
        bool TakeChar = true;
        bool End = false;

        if(specialflags & SPECIAL_ITERATOR_ONNEWLINE_STOP && IsAtNewLine()) {
            ITR_FUNCDEBUG("Stopping to new line");

            if(data->Positions.Start) {
                // Set the last character to two before this
                // (skip the current and " and end there)
                int previouscheck = GetPreviousCharacter();

                // Check do we need to go back 2 characters or just one //
                if((quotes == QUOTETYPE_BOTH &&
                       (previouscheck == '"' || previouscheck == '\'')) ||
                    (quotes == QUOTETYPE_DOUBLEQUOTES && previouscheck == '"') ||
                    (quotes == QUOTETYPE_SINGLEQUOTES && previouscheck == '\'')) {
                    ITR_FUNCDEBUG("Going back over an extra quote character");
                    data->Positions.End = GetPosition() - 2;
                } else {
                    data->Positions.End = GetPosition() - 1;
                }

                ITR_FUNCDEBUG("Ending to new line, end is now: " +
                              Convert::ToString(data->Positions.End));
            }

            SkipLineEnd();
            return ITERATORCALLBACK_RETURNTYPE_STOP;
        }

        int currentcharacter = GetCharacter();

        switch(quotes) {
        case QUOTETYPE_BOTH: {
            if(CurrentFlags & ITERATORFLAG_SET_INSIDE_STRING) {
                // check if we are on the quotes, because we don't want those //
                if(currentcharacter == '"' || currentcharacter == '\'') {
                    // if we aren't ignoring special disallow //
                    if(!(CurrentFlags & ITERATORFLAG_SET_IGNORE_SPECIAL)) {
                        TakeChar = false;
                        ITR_FUNCDEBUG("Found quote character");
                    }
                }

            } else {
                End = true;
                ITR_FUNCDEBUG("Outside quotes");
            }
        } break;
        case QUOTETYPE_SINGLEQUOTES: {
            if(CurrentFlags & ITERATORFLAG_SET_INSIDE_STRING_SINGLE) {
                // check if we are on the quotes, because we don't want those //
                if(currentcharacter == '\'') {
                    // if we aren't ignoring special disallow //
                    if(!(CurrentFlags & ITERATORFLAG_SET_IGNORE_SPECIAL)) {
                        TakeChar = false;
                        ITR_FUNCDEBUG("Found quote character");
                    }
                }

            } else {
                End = true;
                ITR_FUNCDEBUG("Outside quotes");
            }
        } break;
        case QUOTETYPE_DOUBLEQUOTES: {
            if(CurrentFlags & ITERATORFLAG_SET_INSIDE_STRING_DOUBLE) {
                // check if we are on the quotes, because we don't want those //
                if(currentcharacter == '"') {
                    // if we aren't ignoring special disallow //
                    if(!(CurrentFlags & ITERATORFLAG_SET_IGNORE_SPECIAL)) {
                        TakeChar = false;
                        ITR_FUNCDEBUG("Found quote character");
                    }
                }

            } else {
                End = true;
                ITR_FUNCDEBUG("Outside quotes");
            }
        } break;
        }

        // If we have found a character this is on the ending quote //
        if(!TakeChar && data->Positions.Start) {

            // Set the last character to the one before this (skip the " and end there) //
            data->Positions.End = GetPosition() - 1;
            ITR_FUNCDEBUG(
                "On ending quote, end is now: " + Convert::ToString(data->Positions.End));
        }

        if(End) {
            // if we have found at least a character we can end this here //
            if(data->Positions.Start) {
                // Set the last character to two before this
                // (skip the current and " and end there)
                data->Positions.End = GetPosition() - 2;
                ITR_FUNCDEBUG("Ending outside quotes, end is now: " +
                              Convert::ToString(data->Positions));
                return ITERATORCALLBACK_RETURNTYPE_STOP;
            }
        } else if(TakeChar) {
            // check is this first quoted character //
            if(!data->Positions.Start) {
                // first position! //
                data->Positions.Start = GetPosition();
                ITR_FUNCDEBUG(
                    "First character found: " + Convert::ToString(data->Positions.Start));
            }
        }

        return ITERATORCALLBACK_RETURNTYPE_CONTINUE;
    }

    inline ITERATORCALLBACK_RETURNTYPE FindNextNormalCharacterString(
        IteratorPositionData* data, int stopflags, int specialflags)
    {
        bool IsValid = true;

        if(specialflags & SPECIAL_ITERATOR_ONNEWLINE_STOP && IsAtNewLine()) {
            ITR_FUNCDEBUG("Stopping to new line");

            if(data->Positions.Start) {
                // ended //
                data->Positions.End = GetPosition() - 1;
                ITR_FUNCDEBUG("Ending to new line, end is now: " +
                              Convert::ToString(data->Positions.End));
            }

            SkipLineEnd();
            return ITERATORCALLBACK_RETURNTYPE_STOP;
        }

        int currentcharacter = GetCharacter();

        // If set this is invalid inside comments //
        if((specialflags & SPECIAL_ITERATOR_HANDLECOMMENTS_ASSTRING) &&
            (CurrentFlags & ITERATORFLAG_SET_INSIDE_COMMENT)) {
            IsValid = false;
            goto invalidcodelabelunnormalcharacter;
        }

        if((stopflags & UNNORMALCHARACTER_TYPE_LOWCODES ||
               stopflags & UNNORMALCHARACTER_TYPE_WHITESPACE) &&
            !(CurrentFlags & ITERATORFLAG_SET_INSIDE_STRING)) {
            if(currentcharacter <= ' ') {
                IsValid = false;
                goto invalidcodelabelunnormalcharacter;
            }
        }

        if(stopflags & UNNORMALCHARACTER_TYPE_NON_ASCII) {

            if(CurrentFlags & ITERATORFLAG_SET_INSIDE_STRING ||
                !((currentcharacter >= '0' && currentcharacter <= '9') ||
                    (currentcharacter >= 'A' && currentcharacter <= 'Z') ||
                    (currentcharacter >= 'a' && currentcharacter <= 'z'))) {
                IsValid = false;
                goto invalidcodelabelunnormalcharacter;
            }
        }

        if(stopflags & UNNORMALCHARACTER_TYPE_CONTROLCHARACTERS &&
            !(CurrentFlags & ITERATORFLAG_SET_INSIDE_STRING) &&
            !(CurrentFlags & ITERATORFLAG_SET_IGNORE_SPECIAL)) {
            if(((currentcharacter <= '/' && currentcharacter >= '!') ||
                   (currentcharacter <= '@' && currentcharacter >= ':') ||
                   (currentcharacter <= '`' && currentcharacter >= '[') ||
                   (currentcharacter <= '~' && currentcharacter >= '{')) &&
                !(currentcharacter == '_' || currentcharacter == '-')) {
                IsValid = false;
                goto invalidcodelabelunnormalcharacter;
            }
        }

        if(IsValid) {
            // check is this first character //
            if(!data->Positions.Start) {
                // first position! //
                data->Positions.Start = GetPosition();
                ITR_FUNCDEBUG("Started: " + Convert::ToString(data->Positions.Start));
            }

        } else {

        invalidcodelabelunnormalcharacter:


            // check for end //
            if(data->Positions.Start) {
                // ended //
                data->Positions.End = GetPosition() - 1;
                ITR_FUNCDEBUG("End now: " + Convert::ToString(data->Positions.End));
                return ITERATORCALLBACK_RETURNTYPE_STOP;
            }
        }

        return ITERATORCALLBACK_RETURNTYPE_CONTINUE;
    }

    inline ITERATORCALLBACK_RETURNTYPE FindNextNumber(
        IteratorNumberFindData* data, DECIMALSEPARATORTYPE decimal, int specialflags)
    {
        // Check is the current element a part of a number //

        bool IsValid = false;

        if(specialflags & SPECIAL_ITERATOR_ONNEWLINE_STOP && IsAtNewLine()) {
            ITR_FUNCDEBUG("Stopping to new line");

            if(data->Positions.Start) {
                // ended //
                data->Positions.End = GetPosition() - 1;
                ITR_FUNCDEBUG("Ending to new line, end is now: " +
                              Convert::ToString(data->Positions.End));
            }

            SkipLineEnd();
            return ITERATORCALLBACK_RETURNTYPE_STOP;
        }

        // Comments might be skipped //
        if(!(specialflags & SPECIAL_ITERATOR_HANDLECOMMENTS_ASSTRING) ||
            !(CurrentFlags & ITERATORFLAG_SET_INSIDE_COMMENT)) {
            int currentcharacter = GetCharacter();

            if((currentcharacter >= 48) && (currentcharacter <= 57)) {
                // Is a plain old digit //
                IsValid = true;

            } else {
                // Check is it a decimal separator (1 allowed)
                // or a negativity sign in front
                if(currentcharacter == '+' || currentcharacter == '-') {

                    if((data->DigitsFound < 1) && (!data->NegativeFound)) {
                        IsValid = true;
                    }
                    data->NegativeFound = true;
                } else if(((currentcharacter == '.') &&
                              ((decimal == DECIMALSEPARATORTYPE_DOT) ||
                                  (decimal == DECIMALSEPARATORTYPE_BOTH))) ||
                          ((currentcharacter == ',') &&
                              ((decimal == DECIMALSEPARATORTYPE_COMMA) ||
                                  (decimal == DECIMALSEPARATORTYPE_BOTH)))) {
                    if((!data->DecimalFound) && (data->DigitsFound > 0)) {
                        IsValid = true;
                        data->DecimalFound = true;
                    }
                }
            }
        } else {
            ITR_FUNCDEBUG("Ignoring inside a comment");
        }

        if(IsValid) {
            // check is this first digit //
            data->DigitsFound++;
            if(!data->Positions.Start) {
                // first position! //

                data->Positions.Start = GetPosition();
                ITR_FUNCDEBUG("Data started: " + Convert::ToString(data->Positions.Start));
            }

        } else {
            // check for end //
            if(data->Positions.Start) {
                // ended //
                data->Positions.End = GetPosition() - 1;
                ITR_FUNCDEBUG("End now: " + Convert::ToString(data->Positions.End));
                return ITERATORCALLBACK_RETURNTYPE_STOP;
            }
        }
        return ITERATORCALLBACK_RETURNTYPE_CONTINUE;
    }

    inline ITERATORCALLBACK_RETURNTYPE FindUntilEquality(
        IteratorAssignmentData* data, EQUALITYCHARACTER equality, int specialflags)
    {
        // check is current element a valid element //
        bool IsValid = true;
        bool IsStop = false;

        if(specialflags & SPECIAL_ITERATOR_ONNEWLINE_STOP && IsAtNewLine()) {
            ITR_FUNCDEBUG("Stopping to new line");

            SkipLineEnd();
            return ITERATORCALLBACK_RETURNTYPE_STOP;
        }

        // Comments cannot be part of this //
        if((specialflags & SPECIAL_ITERATOR_HANDLECOMMENTS_ASSTRING) &&
            (CurrentFlags & ITERATORFLAG_SET_INSIDE_COMMENT)) {
            // Not valid inside a comment //
            ITR_FUNCDEBUG("Comment skipped");
            IsValid = false;

        } else {

            int charvalue = GetCharacter();

            // Skip if this is a space //
            if(charvalue < 33) {
                // Not allowed in a name //
                ITR_FUNCDEBUG("Whitespace skipped");
                IsValid = false;
            }

            if(equality == EQUALITYCHARACTER_TYPE_ALL) {
                // check for all possible value separators //
                if(charvalue == '=' || charvalue == ':') {

                    if(!(CurrentFlags & ITERATORFLAG_SET_IGNORE_SPECIAL)) {
                        // If ignored don't stop //
                        ITR_FUNCDEBUG("Found = or :");
                        IsStop = true;
                    }
                }
            } else if(equality == EQUALITYCHARACTER_TYPE_EQUALITY) {
                // Check for an equality sign //
                if(charvalue == '=') {
                    if(!(CurrentFlags & ITERATORFLAG_SET_IGNORE_SPECIAL)) {
                        ITR_FUNCDEBUG("Found =");
                        IsStop = true;
                    }
                }
            } else if(equality == EQUALITYCHARACTER_TYPE_DOUBLEDOTSTYLE) {
                // Check does it match the characters //
                if(charvalue == ':') {
                    if(!(CurrentFlags & ITERATORFLAG_SET_IGNORE_SPECIAL)) {
                        ITR_FUNCDEBUG("Found :");
                        IsStop = true;
                    }
                }
            }
        }

        if(!IsStop) {
            // end if end already found //
            if(data->SeparatorFound) {
                ITR_FUNCDEBUG("Found stop");
                return ITERATORCALLBACK_RETURNTYPE_STOP;
            }
        } else {
            data->SeparatorFound = true;
            IsValid = false;
        }

        if(IsValid) {
            // Check is this the first character //
            if(!data->Positions.Start) {
                // first position! //
                data->Positions.Start = GetPosition();
                ITR_FUNCDEBUG("Data started: " + Convert::ToString(data->Positions.Start));

            } else {
                // Set end to this valid character //
                data->Positions.End = GetPosition();
                ITR_FUNCDEBUG("End now: " + Convert::ToString(data->Positions.End));
            }
        }


        return ITERATORCALLBACK_RETURNTYPE_CONTINUE;
    }

    inline ITERATORCALLBACK_RETURNTYPE SkipSomething(
        IteratorCharacterData& data, const int additionalskip, const int specialflags)
    {
        if(specialflags & SPECIAL_ITERATOR_ONNEWLINE_STOP && IsAtNewLine()) {
            ITR_FUNCDEBUG("Stopping to new line");

            SkipLineEnd();
            return ITERATORCALLBACK_RETURNTYPE_STOP;
        }

        // We can probably always skip inside a comment //
        if((specialflags & SPECIAL_ITERATOR_HANDLECOMMENTS_ASSTRING) &&
            (CurrentFlags & ITERATORFLAG_SET_INSIDE_COMMENT)) {
            return ITERATORCALLBACK_RETURNTYPE_CONTINUE;
        }

        // We can just return if we are inside a string //
        if(CurrentFlags & ITERATORFLAG_SET_INSIDE_STRING) {
            return ITERATORCALLBACK_RETURNTYPE_STOP;
        }

        // Check does the character match what is being skipped //
        int curchara = GetCharacter();

        if(additionalskip & UNNORMALCHARACTER_TYPE_LOWCODES) {
            if(curchara <= 32)
                return ITERATORCALLBACK_RETURNTYPE_CONTINUE;
        }

        if(curchara == data.CharacterToUse) {
            // We want to skip it //
            return ITERATORCALLBACK_RETURNTYPE_CONTINUE;
        }

        // didn't match to be skipped characters //
        return ITERATORCALLBACK_RETURNTYPE_STOP;
    }

    inline ITERATORCALLBACK_RETURNTYPE FindUntilSpecificCharacter(
        IteratorFindUntilData* data, int character, int specialflags)
    {
        // Can this character be added //
        bool ValidChar = true;

        if(specialflags & SPECIAL_ITERATOR_ONNEWLINE_STOP && IsAtNewLine()) {
            ITR_FUNCDEBUG("Stopping to new line");

            if(data->Positions.Start) {
                // This should be fine to get here //
                data->Positions.End = GetPosition() - 1;
                ITR_FUNCDEBUG("Ending to new line, end is now: " +
                              Convert::ToString(data->Positions.End));
            }

            // Make sure to not return until end of the whole string //
            data->NewLineBreak = true;

            SkipLineEnd();
            return ITERATORCALLBACK_RETURNTYPE_STOP;
        }

        int tmpchara = GetCharacter();

        // We can just continue if we are inside a string //
        if(!(CurrentFlags & ITERATORFLAG_SET_INSIDE_STRING) &&
            !((specialflags & SPECIAL_ITERATOR_HANDLECOMMENTS_ASSTRING) &&
                (CurrentFlags & ITERATORFLAG_SET_INSIDE_COMMENT))) {
            // Check did we encounter stop character //
#ifdef ALLOW_DEBUG
            if(DebugMode) {

                std::string value = "Trying to match: ";
                utf8::append(tmpchara, std::back_inserter(value));
                value += "==" + Convert::ToString(tmpchara);

                Logger::Get()->Write("Iterator: procfunc: " + value);
            }
#endif // ALLOW_DEBUG

            if(tmpchara == character) {
                // Skip if ignoring special characters //
                if(!(CurrentFlags & ITERATORFLAG_SET_IGNORE_SPECIAL)) {
                    // Not valid character //
                    ValidChar = false;
                    ITR_FUNCDEBUG("Found match");
                    // We must have started to encounter the stop character //
                    if(data->Positions.Start) {
                        // We encountered the stop character //
                        data->FoundEnd = true;
                        ITR_FUNCDEBUG("Encountered end");
                    }
                }
            }
        } else {
#ifdef ALLOW_DEBUG
            if((CurrentFlags & ITERATORFLAG_SET_INSIDE_STRING)) {
                ITR_FUNCDEBUG("Ignoring inside string");
            }
            if((specialflags & SPECIAL_ITERATOR_HANDLECOMMENTS_ASSTRING) &&
                (CurrentFlags & ITERATORFLAG_SET_INSIDE_COMMENT)) {
                ITR_FUNCDEBUG("Ignoring inside comment");
            }
#endif // _DEBUG
        }

        if(ValidChar) {
            // valid character set start if not already set //
            if(!data->Positions.Start) {
                data->Positions.Start = GetPosition();
                data->Positions.End = data->Positions.Start;
                ITR_FUNCDEBUG("Data started: " + Convert::ToString(data->Positions.Start));
            }
            return ITERATORCALLBACK_RETURNTYPE_CONTINUE;
        }
        // let's stop if we have found something //
        if(data->Positions.Start) {
            // This should be fine to get here //
            data->Positions.End = GetPosition() - 1;
            ITR_FUNCDEBUG("Ending here: " + Convert::ToString(data->Positions.End));
            return ITERATORCALLBACK_RETURNTYPE_STOP;
        }

        // haven't found anything, we'll need to find something //
        return ITERATORCALLBACK_RETURNTYPE_CONTINUE;
    }

    inline ITERATORCALLBACK_RETURNTYPE FindUntilNewLine(IteratorFindUntilData* data)
    {
        // Continue if the current character is a new line character //

        // All line separator characters should be here //
        if(IsAtNewLine()) {

            if(!data->FoundEnd) {
                // Ignore the first new line //
                data->FoundEnd = true;
                ITR_FUNCDEBUG("Ignoring first newline character");
                goto positionisvalidlabelstringiteratorfindnewline;
            }

            // This is a new line character //
            ITR_FUNCDEBUG("Found newline character");

            // End before this character //
            data->Positions.End = GetPosition() - 1;
            ITR_FUNCDEBUG("Ending here: " + Convert::ToString(data->Positions));

            SkipLineEnd();
            return ITERATORCALLBACK_RETURNTYPE_STOP;
        }

    positionisvalidlabelstringiteratorfindnewline:

        // Set position //
        if(!data->Positions.Start) {

            // End before this character //
            data->Positions.Start = GetPosition();
            data->FoundEnd = true;
            ITR_FUNCDEBUG("Data started: " + Convert::ToString(data->Positions));
        }

        return ITERATORCALLBACK_RETURNTYPE_CONTINUE;
    }

    inline ITERATORCALLBACK_RETURNTYPE FindInMatchingParentheses(
        IteratorNestingLevelData* data, int left, int right, int specialflags)
    {
        // Ignore if ignoring special characters //
        if(!(CurrentFlags & ITERATORFLAG_SET_IGNORE_SPECIAL) &&
            !(CurrentFlags & ITERATORFLAG_SET_INSIDE_STRING)) {
            if(specialflags & SPECIAL_ITERATOR_ONNEWLINE_STOP && IsAtNewLine()) {

                // Invalid, always //
                return ITERATORCALLBACK_RETURNTYPE_STOP;
            }

            int currentcharacter = GetCharacter();

            // Nesting level starts //
            if(currentcharacter == left) {

                ++data->NestingLevel;

                if(data->NestingLevel > 1) {

                    // There where multiple lefts in a row, like "[[[...]]]"
                    goto isinsidevalidleftrightpair;
                }

                return ITERATORCALLBACK_RETURNTYPE_CONTINUE;
            }

            // One nesting level is ending //
            if(currentcharacter == right) {

                --data->NestingLevel;

                if(data->NestingLevel == 0) {

                    data->Positions.End = GetPosition() - 1;
                    return ITERATORCALLBACK_RETURNTYPE_STOP;
                }
            }
        }

    isinsidevalidleftrightpair:

        if(!data->Positions.Start && data->NestingLevel > 0) {

            data->Positions.Start = GetPosition();
        }

        return ITERATORCALLBACK_RETURNTYPE_CONTINUE;
    }



    template<class AcceptStr>
    inline ITERATORCALLBACK_RETURNTYPE FindUntilSequence(
        IteratorUntilSequenceData<AcceptStr>* data, int specialflags)
    {
        // First check if this is a line end //
        int curcharacter = GetCharacter();

        if(specialflags & SPECIAL_ITERATOR_ONNEWLINE_STOP && IsAtNewLine()) {

            // Set the end to one before this, if found any //
            if(data->Positions.Start) {

                data->Positions.End = GetPosition() - 1;
            }

            SkipLineEnd();
            return ITERATORCALLBACK_RETURNTYPE_STOP;
        }

        // We may not be inside strings nor comments for checking //
        if(!(CurrentFlags & ITERATORFLAG_SET_INSIDE_STRING) &&
            (!(CurrentFlags & ITERATORFLAG_SET_INSIDE_COMMENT) ||
                !(specialflags & SPECIAL_ITERATOR_HANDLECOMMENTS_ASSTRING))) {
            // Check do we match the current position //
            if(curcharacter == data->StringToMatch[data->CurMatchedIndex]) {

                // Found a matching character //

                // Move to next match position and don't yet verify if this is a valid
                // character
                ++data->CurMatchedIndex;

                if(data->CurMatchedIndex >= data->StringToMatch.size()) {

                    // End found //
                    return ITERATORCALLBACK_RETURNTYPE_STOP;
                }

                return ITERATORCALLBACK_RETURNTYPE_CONTINUE;
            }
        }

        // Go back to beginning of matching //
        data->CurMatchedIndex = 0;

        // All is fine //
        if(!data->Positions.Start) {

            data->Positions.Start = GetPosition();
            data->Positions.End = data->Positions.Start;
        } else {

            // This might be poisonous to performance, but this gets the job done //
            data->Positions.End = GetPosition();
        }

        return ITERATORCALLBACK_RETURNTYPE_CONTINUE;
    }
};

} // namespace Leviathan
#undef ALLOW_DEBUG

#ifdef LEAK_INTO_GLOBAL
using Leviathan::StringIterator;
#endif
