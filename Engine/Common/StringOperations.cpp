#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_STRINGOPERATIONS
#include "StringOperations.h"
#endif
using namespace Leviathan;
// ------------------------------------ //

template<> const std::wstring Leviathan::StringConstants<std::wstring,
                                                         wchar_t>::WindowsLineSeparator = L"\r\n";

template<> const std::wstring Leviathan::StringConstants<std::wstring,
                                                         wchar_t>::UniversalLineSeparator = L"\n";


template<> const std::string Leviathan::StringConstants<std::string,
                                                        char>::WindowsLineSeparator = "\r\n";
template<> const std::string Leviathan::StringConstants<std::string,
                                                        char>::UniversalLineSeparator = "\n";
