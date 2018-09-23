// ------------------------------------ //
#include "StringIterator.h"

#include "utf8/checked.h"
using namespace Leviathan;
using namespace std;
// ------------------------------------ //
DLLEXPORT StringIterator::StringIterator() {}

DLLEXPORT Leviathan::StringIterator::StringIterator(
    std::unique_ptr<StringDataIterator>&& iterator) :
    HandlesDelete(true),
    DataIterator(iterator.release())
{}

DLLEXPORT Leviathan::StringIterator::StringIterator(const string& text) :
    HandlesDelete(true), DataIterator(new StringClassDataIterator<string>(text))
{}

DLLEXPORT Leviathan::StringIterator::StringIterator(const wstring& text) :
    HandlesDelete(true), DataIterator(new StringClassDataIterator<wstring>(text))
{}

DLLEXPORT Leviathan::StringIterator::StringIterator(const wstring* text) :
    HandlesDelete(true), DataIterator(new StringClassPointerIterator<wstring>(text))
{}

DLLEXPORT Leviathan::StringIterator::StringIterator(const string* text) :
    HandlesDelete(true), DataIterator(new StringClassPointerIterator<string>(text))
{}

DLLEXPORT Leviathan::StringIterator::~StringIterator()
{
    if(HandlesDelete) {

        SAFE_DELETE(DataIterator);
    }
}
// ------------------------------------ //
DLLEXPORT void StringIterator::ReInit(std::unique_ptr<StringDataIterator>&& iterator)
{
    // Remove the last iterator //
    if(HandlesDelete) {

        SAFE_DELETE(DataIterator);
    }

    ITR_COREDEBUG("ReInit")

    HandlesDelete = true;
    DataIterator = iterator.release();

    // Reset everything //
    CurrentCharacter = -1;
    CurrentStored = false;


    // Clear the flags //
    CurrentFlags = 0;
}

DLLEXPORT void Leviathan::StringIterator::ReInit(const wstring& text)
{
    ReInit(std::make_unique<StringClassDataIterator<wstring>>(text));
}

DLLEXPORT void Leviathan::StringIterator::ReInit(const string& text)
{
    ReInit(std::make_unique<StringClassDataIterator<string>>(text));
}

DLLEXPORT void Leviathan::StringIterator::ReInit(const wstring* text)
{
    ReInit(std::make_unique<StringClassPointerIterator<wstring>>(text));
}

DLLEXPORT void Leviathan::StringIterator::ReInit(const string* text)
{
    ReInit(std::make_unique<StringClassPointerIterator<string>>(text));
}
// ------------------------------------ //
DLLEXPORT int Leviathan::StringIterator::GetPreviousCharacter()
{
    int tmpval = -1;
    if(!DataIterator->GetPreviousCharacter(tmpval)) {
        // Darn //
        ITR_COREDEBUG("Failed to get previous character");
        return 0;
    }

    ITR_COREDEBUG("Peek back char: (" + Convert::CodePointToUtf8(tmpval) + ")");
    return tmpval;
}
// ------------------------------------ //
DLLEXPORT void Leviathan::StringIterator::SkipLineEnd()
{
    ITR_COREDEBUG("Skip line end");

    const auto current = GetCharacter(0);
    const auto next = GetCharacter(1);

    // Move past the current new line //
    MoveToNext();

    // Skip multi part new lines //
    if(StringOperations::IsLineTerminator(current, next))
        MoveToNext();
}
