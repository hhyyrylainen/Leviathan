#pragma once
// ------------------------------------ //
#include "Define.h"
// ------------------------------------ //


namespace Leviathan{

//! Interface that wraps string for iteration with StringIterator
//!
//! Subclasses allow different types of data sources to be used with this class
class StringDataIterator{
public:
    DLLEXPORT StringDataIterator();
    DLLEXPORT virtual ~StringDataIterator();

    // Implement these in all subclasses to wrap the input for iteration //

    //! \brief Gets the next character unicode code point (usually an ascii value)
    //! \param forward Defines how many characters past the current position the wanted
    //! character is. This is usually 0 or 1
    //! \note This is used to get the next character but also used for peeking so this should
    //! not increment the underlying iterator
    //! \return Returns true when the input is still valid
    virtual bool GetNextCharCode(int &codepointreceiver, size_t forward) = 0;

    //! \brief Gets the previous character code point
    //! \return True when succeeded false when this is the first character and there is
    //! nothing before it
    //! \see GetNextCharCode
    virtual bool GetPreviousCharacter(int &receiver) = 0;

    //! \brief Moves the iterator forward
    virtual void MoveToNextCharacter() = 0;


    // These are for getting the results //
    // If the internal string doesn't support efficient conversions false should be returned
    // and nothing set


    DLLEXPORT virtual bool ReturnSubString(size_t startpos, size_t endpos, 
        std::string &receiver);
    DLLEXPORT virtual bool ReturnSubString(size_t startpos, size_t endpos, 
        std::wstring &receiver);


    //! \brief Gets the position of the iterator, for use with ReturnSubString and others
    virtual size_t CurrentIteratorPosition() const = 0;

    //! \brief Gets the last valid index of the underlying string
    //! (not the last character but the last byte)
    virtual size_t GetLastValidIteratorPosition() const = 0;


    //! \brief Returns true when the iterator is still valid
    virtual bool IsPositionValid() const = 0;


    // Basic functions that should all be the same //

    //! Returns the 0 based character number (NOT position, number of characters)
    DLLEXPORT virtual size_t GetCurrentCharacterNumber() const;

    //! Returns the 1 based line number
    DLLEXPORT virtual size_t GetCurrentLineNumber() const;


protected:

    //! Updates CurrentLineNumber if currently on a line break
    DLLEXPORT void CheckLineChange();

protected:

    //! Must be the current character number, not the number of bytes for example one utf8
    //! character might be 4 bytes
    size_t CurrentCharacterNumber;

    //! The current line number, the amount of \n characters 
    size_t CurrentLineNumber;


};


//! Iterator for string types
template<class STRType>
    class StringClassDataIterator : public StringDataIterator{
    typedef size_t ITType;
public:

    //! Wraps a string reference for StringIterator
    //! \note The string should not be changed while the iterator is used
    StringClassDataIterator(const STRType &str) : OurString(str), Current(0), End(str.size()){
        // If the first character is a newline the line number needs to be incremented immediately //
        if(OurString.size() && OurString[0] == '\n'){

            ++CurrentLineNumber;
        }

    }

    virtual bool GetNextCharCode(int &codepointreceiver, size_t forward){
        if(Current+forward >= End)
            return false;
        // Copy the character //
        codepointreceiver = static_cast<int>(OurString[Current+forward]);
        return true;
    }

    virtual bool GetPreviousCharacter(int &receiver){
        if(Current-1 >= End)
            return false;

        // Copy the character //
        receiver = static_cast<int>(OurString[Current-1]);
        return true;
    }

    virtual void MoveToNextCharacter(){
        ++Current;
        // Don't forget to increment these //
        ++CurrentCharacterNumber;
        // There might be a better way to check this //
        CheckLineChange();
    }

    virtual bool ReturnSubString(size_t startpos, size_t endpos, STRType &receiver){
        if(startpos >= End || endpos >= End || startpos > endpos)
            return false;

        receiver = OurString.substr(startpos, endpos-startpos+1);
        return true;
    }

    virtual size_t CurrentIteratorPosition() const{
        return Current;
    }

    virtual bool IsPositionValid() const{
        if(Current < End)
            return true;
        return false;
    }

    virtual size_t GetLastValidIteratorPosition() const{

        return End-1;
    }



protected:

    STRType OurString;

    //! The current position of the iterator
    ITType Current;
    //! The end of the string
    ITType End;
};


//! Iterator that doesn't hold own copy of a string
template<class STRType>
    class StringClassPointerIterator : public StringDataIterator{
    typedef size_t ITType;
public:

    //! Wraps a string reference for StringIterator
    //! \note The string should not be changed while the iterator is used
    StringClassPointerIterator(const STRType* str) : OurString(str), Current(0){
        End = str ? str->size(): 0;

        // If the first character is a newline the line number needs to be incremented immediately //
        if(OurString && OurString->size() && OurString->at(0) == '\n'){

            ++CurrentLineNumber;
        }
    }

    virtual bool GetNextCharCode(int &codepointreceiver, size_t forward){
        if(Current+forward >= End)
            return false;
        // Copy the character //
        codepointreceiver = static_cast<int>(OurString->at(Current+forward));
        return true;
    }

    virtual bool GetPreviousCharacter(int &receiver){
        if(Current-1 >= End)
            return false;

        // Copy the character //
        receiver = static_cast<int>(OurString->at(Current-1));
        return true;
    }

    virtual void MoveToNextCharacter(){
        ++Current;
        // Don't forget to increment these //
        ++CurrentCharacterNumber;

        CheckLineChange();
    }

    virtual bool ReturnSubString(size_t startpos, size_t endpos, STRType &receiver){
        if(startpos >= End || endpos >= End || startpos > endpos)
            return false;

        receiver = OurString->substr(startpos, endpos-startpos+1);
        return true;
    }

    virtual size_t CurrentIteratorPosition() const{
        return Current;
    }

    virtual bool IsPositionValid() const{
        if(Current < End)
            return true;
        return false;
    }

    virtual size_t GetLastValidIteratorPosition() const{

        return End-1;
    }

protected:

    const STRType* OurString;

    //! The current position of the iterator
    ITType Current;
    //! The end of the string
    ITType End;
};

//! Raw pointer utf8 iterator
class UTF8PointerDataIterator : public StringDataIterator{
public:

    //! end points 1 past the end of the data
    //! "this is my string\0"
    //!  ^-begin          ^-end
    //! meaning the null terminator is optional
    //! \note Child classes may pass null pointers here as long as they call CheckLineChange
    //! in their constructors (to not miss line changes if the first character is a
    //! line change, see UTF8DataIterator(const std::string &str) constructor)
    DLLEXPORT UTF8PointerDataIterator(const char* begin, const char* end);

    //! Helper for creating from strings
    DLLEXPORT UTF8PointerDataIterator(const std::string &fromstr);
    
    DLLEXPORT virtual bool GetNextCharCode(int &codepointreceiver, size_t forward);

    DLLEXPORT virtual bool GetPreviousCharacter(int &receiver);

    DLLEXPORT virtual void MoveToNextCharacter();

    DLLEXPORT virtual size_t CurrentIteratorPosition() const;

    DLLEXPORT virtual bool IsPositionValid() const;

    DLLEXPORT virtual size_t GetLastValidIteratorPosition() const;

    DLLEXPORT virtual bool ReturnSubString(size_t startpos, size_t endpos, 
        std::string &receiver);

protected:

    //! The current position of the iterator
    const char* Current;
    //! The end of the string
    const char* End;

    //! The starting point for distance checking
    const char* BeginPos;
    
};


//! Unicode iterator for utf8 on top of string
class UTF8DataIterator : public UTF8PointerDataIterator{
    typedef std::string::iterator ITType;
public:

    //! Wraps an utf8 encoded string for StringIterator
    DLLEXPORT UTF8DataIterator(const std::string &str);

protected:

    std::string OurString;
};


}

#ifdef LEAK_INTO_GLOBAL
using Leviathan::UTF8DataIterator;
#endif

