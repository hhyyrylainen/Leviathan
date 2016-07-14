// ------------------------------------ //
#include "StringOperations.h"
using namespace Leviathan;
// ------------------------------------ //

template<>
    DLLEXPORT void StringOperations::MakeString(std::wstring &str, const char* characters,
        size_t count)
{
    // Skip copying null terminator
    const size_t copysize = count - 1;
    str.resize(copysize);

    for (size_t i = 0; i < copysize; ++i)
        str[i] = (wchar_t)characters[i];
}
