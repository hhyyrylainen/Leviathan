#include "Include.h"
// ------------------------------------ //
#include "StringDataIterator.h"
#include "Common/StringOperations.h"

#if !defined(ALTERNATIVE_EXCEPTIONS_FATAL) || defined(ALLOW_INTERNAL_EXCEPTIONS)
#include "utf8/checked.h"
#else
#include "utf8/unchecked.h"
#endif //ALTERNATIVE_EXCEPTIONS_FATAL
using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT Leviathan::StringDataIterator::StringDataIterator() :
    CurrentCharacterNumber(0), CurrentLineNumber(1)
{

}

DLLEXPORT Leviathan::StringDataIterator::~StringDataIterator(){

}
// ------------------------------------ //
DLLEXPORT bool Leviathan::StringDataIterator::ReturnSubString(size_t startpos, size_t endpos,
    std::string &receiver)
{
    DEBUG_BREAK;
    LEVIATHAN_ASSERT(0, "StringDataIterator doesn't support getting with type: string, "
        "make sure your provided data source string type is the same as the "
        "request template type");
    return false;
}

DLLEXPORT bool Leviathan::StringDataIterator::ReturnSubString(size_t startpos, size_t endpos,
    std::wstring &receiver)
{
    DEBUG_BREAK;
    LEVIATHAN_ASSERT(0, "StringDataIterator doesn't support getting with type: wstring, make "
        "sure your provided data source string type is the same as the request template type");
    return false;
}
// ------------------------------------ //
DLLEXPORT size_t Leviathan::StringDataIterator::GetCurrentCharacterNumber() const{
    return CurrentCharacterNumber;
}

DLLEXPORT size_t Leviathan::StringDataIterator::GetCurrentLineNumber() const{
    return CurrentLineNumber;
}

DLLEXPORT void Leviathan::StringDataIterator::CheckLineChange() {

    int32_t checkchar;

    if (GetNextCharCode(checkchar, 0) && StringOperations::IsLineTerminator(checkchar)) {

        // Check for multiline //
        int32_t evenfurthercharacter;
        if (!GetNextCharCode(evenfurthercharacter, 1) ||
            !StringOperations::IsLineTerminator(checkchar, evenfurthercharacter))
        {
            ++CurrentLineNumber;
        }
    }
}

// ------------------------------------ //
// UTF8PointerDataIterator
DLLEXPORT UTF8PointerDataIterator::UTF8PointerDataIterator(const char* begin, const char* end) :
    Current(begin), End(end), BeginPos(Current)
{
    // If the first character is a newline the line number needs to be
    // incremented immediately    
    if(Current)
        CheckLineChange();
}

DLLEXPORT UTF8PointerDataIterator::UTF8PointerDataIterator(const std::string &fromstr) :
    UTF8PointerDataIterator(fromstr.c_str(), fromstr.c_str() + fromstr.size())
{

}

DLLEXPORT bool UTF8PointerDataIterator::GetNextCharCode(int &codepointreceiver, 
    size_t forward)
{
    // Check is it out of range
    if(Current + forward >= End)
        return false;

    // We can just peek the next character if forward is 0 //
    if(!forward){

    #if !defined(ALTERNATIVE_EXCEPTIONS_FATAL) || defined(ALLOW_INTERNAL_EXCEPTIONS)
        codepointreceiver = utf8::peek_next(Current, End);
    #else
        codepointreceiver = utf8::unchecked::peek_next(Current);

        // receiver might be full of garbage at this point //


    #endif //ALTERNATIVE_EXCEPTIONS_FATAL
        return true;
    }

    // UTF8 string use the utf8 iterating functions //
    auto shouldbepos = Current;

#if !defined(ALTERNATIVE_EXCEPTIONS_FATAL) || defined(ALLOW_INTERNAL_EXCEPTIONS)

    utf8::advance(shouldbepos, forward, End);

    if (shouldbepos == End)
        return false;

    codepointreceiver = utf8::next(shouldbepos, End);
#else
    utf8::unchecked::advance(shouldbepos, forward);

    if (shouldbepos == End)
        return false;

    codepointreceiver = utf8::unchecked::next(shouldbepos);

    LEVIATHAN_ASSERT(shouldbepos != End, "GetNextCharCode next moved past the end");

#endif //ALTERNATIVE_EXCEPTIONS_FATAL

    return true;
}

DLLEXPORT bool UTF8PointerDataIterator::GetPreviousCharacter(int &receiver){
    
    // Try to get the prior code point //
    auto shouldbepos = Current;

#if !defined(ALTERNATIVE_EXCEPTIONS_FATAL) || defined(ALLOW_INTERNAL_EXCEPTIONS)

    try{
        // Try to copy the previous code point into the receiver //
        receiver = utf8::prior(shouldbepos, BeginPos);

    } catch(const utf8::not_enough_room&){
        return false;
    } catch(const utf8::invalid_utf8&){
        return false;
    }

#else
    receiver = utf8::unchecked::prior(shouldbepos);

    if (shouldbepos == End)
        return false;

#endif //ALTERNATIVE_EXCEPTIONS_FATAL


    // If it didn't throw it worked //
    return true;
}
// ------------------------------------ //
DLLEXPORT void UTF8PointerDataIterator::MoveToNextCharacter(){

    if (!IsPositionValid())
        return;

    // We need to move whole code points //
#if !defined(ALTERNATIVE_EXCEPTIONS_FATAL) || defined(ALLOW_INTERNAL_EXCEPTIONS)

    utf8::advance(Current, 1, End);

#else
    utf8::unchecked::advance(Current, 1);

#endif //ALTERNATIVE_EXCEPTIONS_FATAL

    // Don't forget to increment these //
    ++CurrentCharacterNumber;

    // Return if position is not valid //
    if(!IsPositionValid())
        return;

    // There might be a better way to check this //
    CheckLineChange();
}
// ------------------------------------ //
DLLEXPORT size_t UTF8PointerDataIterator::CurrentIteratorPosition() const{
    return std::distance(BeginPos, Current);
}

DLLEXPORT bool UTF8PointerDataIterator::IsPositionValid() const{
    return Current != End;
}
// ------------------------------------ //
DLLEXPORT size_t UTF8PointerDataIterator::GetLastValidIteratorPosition() const{
    return static_cast<size_t>((End - 1) - BeginPos);
}
// ------------------------------------ //
DLLEXPORT bool UTF8PointerDataIterator::ReturnSubString(size_t startpos, size_t endpos,
    std::string &receiver)
{
    if(startpos >= static_cast<size_t>(End - BeginPos) ||
        endpos >= static_cast<size_t>(End - BeginPos) || startpos > endpos)
    {
        return false;
    }
    
    receiver = std::string(BeginPos + startpos, (endpos - startpos) + 1);
    return true;
}

    

// ------------------------------------ //
// UTF8DataIterator
DLLEXPORT UTF8DataIterator::UTF8DataIterator(const std::string &str) :
    UTF8PointerDataIterator(nullptr, nullptr), OurString(str)
{
    // This is a bit hacky
    Current = OurString.c_str();
    End = Current + OurString.size();
    BeginPos = Current;
    

    // If the first character is a newline the line number needs to be
    // incremented immediately
    CheckLineChange();
}
