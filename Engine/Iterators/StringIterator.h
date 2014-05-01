#ifndef LEVIATHAN_STRINGITERATOR
#define LEVIATHAN_STRINGITERATOR
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "StringDataIterator.h"
#include "IteratorData.h"


namespace Leviathan{


	enum QUOTETYPE {QUOTETYPE_DOUBLEQUOTES, QUOTETYPE_SINGLEQUOTES, QUOTETYPE_BOTH};
	enum DECIMALSEPARATORTYPE {DECIMALSEPARATORTYPE_DOT, DECIMALSEPARATORTYPE_COMMA, DECIMALSEPARATORTYPE_BOTH, DECIMALSEPARATORTYPE_NONE};

	enum UNNORMALCHARACTER {UNNORMALCHARACTER_TYPE_NON_ASCII = 0x1,
		UNNORMALCHARACTER_TYPE_CONTROLCHARACTERS = 0x2,
		UNNORMALCHARACTER_TYPE_WHITESPACE = 0x4,
		UNNORMALCHARACTER_TYPE_LOWCODES = 0x8,
		UNNORMALCHARACTER_TYPE_NON_NAMEVALID = 0x10,
		UNNORMALCHARACTER_TYPE_LINEEND = 0x20};

	enum EQUALITYCHARACTER {EQUALITYCHARACTER_TYPE_EQUALITY, EQUALITYCHARACTER_TYPE_DOUBLEDOTSTYLE, EQUALITYCHARACTER_TYPE_ALL};

	enum ITERATORCALLBACK_RETURNTYPE {ITERATORCALLBACK_RETURNTYPE_STOP, ITERATORCALLBACK_RETURNTYPE_CONTINUE};


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


	typedef ITERATORCALLBACK_RETURNTYPE (*IteratorWstrCallBack)(StringIterator* instance, Object* IteratorData, int parameters);



	//! Iterator class for getting parts of a string
	class StringIterator{
	public:
		//! \brief Creates a iterator from the iterating object
		//! \param TakesOwnership Set to true when iterator should be deleted by this object
		DLLEXPORT StringIterator(StringDataIterator* iterator, bool TakesOwnership = false);	

		//! \brief Helper constructor for common string type
		DLLEXPORT StringIterator(const wstring &text);
		//! \brief Helper constructor for common string type
		DLLEXPORT StringIterator(const string &text);

		DLLEXPORT virtual ~StringIterator();

		//! \brief Changes the current iterator to the new iterator and goes to the beginning
		DLLEXPORT void ReInit(StringDataIterator* iterator, bool TakesOwnership = false);
		//! \brief Helper function for ReInit for common string type
		DLLEXPORT void ReInit(const wstring& text);
		//! \brief Helper function for ReInit for common string type
		DLLEXPORT void ReInit(const string& text);

		// Iterating functions //

		//! \brief Gets the next string in quotes
		//!
		//! This function will skip until it finds a quote (either " or ' specified by quotes) and then returns the content inside
		//! \return The string found or NULL
		template<class RStrType>
		DLLEXPORT unique_ptr<RStrType> GetStringInQuotes(QUOTETYPE quotes);

		//! \brief Gets the next number
		//!
		//! This function will skip until it finds a number and returns the number string according to the decimal parameter.
		//! If the type is DECIMALSEPARATORTYPE_NONE decimal numbers are only read until the dot
		//! \return The string found or NULL
		template<class RStrType>
		DLLEXPORT unique_ptr<RStrType> GetNextNumber(DECIMALSEPARATORTYPE decimal);

		//! \brief Gets the next sequence of characters according to stopcaseflags
		//! \param stopcaseflags Specifies until what type of characters this string is read.
		//! Should be created by using UNNORMALCHARACTER as bit flags inside the argument int
		//! \return The string found or NULL
		template<class RStrType>
		DLLEXPORT unique_ptr<RStrType> GetNextCharacterSequence(int stopcaseflags);

		//! \brief Gets the string that is before the equality assignment
		//!
		//! This function will read until either : or = is encountered specified by stopcase
		//! \return The string found or NULL
		template<class RStrType>
		DLLEXPORT unique_ptr<RStrType> GetUntilEqualityAssignment(EQUALITYCHARACTER stopcase);

		//! \brief Gets all characters until the end
		//! \return The string found or NULL if the read position is invalid
		template<class RStrType>
		DLLEXPORT unique_ptr<RStrType> GetUntilEnd();

		//! \brief Gets characters until a character or nothing if the specified character is not found
		//!
		//! This function will read until charactertolookfor and return the string without charactertolookfor, or if not found nothing
		//! \param charactertolookfor The code point to look for
		//! \return The string found or NULL
		//! \see GetUntilNextCharacterOrAll
		template<class RStrType>
		DLLEXPORT unique_ptr<RStrType> GetUntilNextCharacterOrNothing(int charactertolookfor);

		//! \brief Gets characters until a character or all remaining characters
		//!
		//! This function will read until charactertolookfor and return the string without charactertolookfor, or if not found until the end
		//! \param charactertolookfor The code point to look for
		//! \return The string found or NULL if there are no valid characters left
		//! \see GetUntilNextCharacterOrAll GetUntilEnd
		template<class RStrType>
		DLLEXPORT unique_ptr<RStrType> GetUntilNextCharacterOrAll(int charactertolookfor);

		//! \brief Skips until characters that are not whitespace are found
		//! \see SkipCharacters
		DLLEXPORT void SkipWhiteSpace();

		//! \brief Skips until chartoskip doesn't match the current character
		//! \param chartoskip The code point to skip
		//! \see SkipWhiteSpace
		DLLEXPORT void SkipCharacters(int chartoskip);

		// Utility functions //

#ifdef _DEBUG
		//! \brief When set to true prints lots of debug output
		DLLEXPORT void SetDebugMode(const bool &mode);
#endif

		//! \brief Returns the current reading position
		//! \note This might be expensiveish operation based on the underlying StringDataIterator class (mostly expensive for UTF8 strings)
		DLLEXPORT size_t GetPosition();

		//! \brief Gets the character in the position current + forward
		DLLEXPORT int GetCharacter(size_t forward = 0);

		//! \brief Skips the current character and moves to the next
		//! \return True when there is a valid character or false if the end has already been reached
		DLLEXPORT bool MoveToNext();

		//! \brief Returns true when the read position is valid
		DLLEXPORT bool IsOutOfBounds();


	private:


		inline ITERATORCALLBACK_RETURNTYPE HandleSpecialCharacters();
		inline ITERATORCALLBACK_RETURNTYPE CheckActiveFlags();


		// ------------------------------------ //
		//! Set when we should delete DataIterator
		bool HandlesDelete;

#ifdef _DEBUG
		//! Controls debug output printing
		bool DebugMode;
#endif
		
		//! Wraps the underlying string
		StringDataIterator* DataIterator;

		//! Currently active flags
		//! Bit field of ITERATORFLAG_SET enum values
		int CurrentFlags;


	protected:

		// Iteration functions //



	};

}
#endif
