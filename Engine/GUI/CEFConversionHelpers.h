// Leviathan Game Engine
// Copyright (c) 2012-2018 Henri Hyyryläinen
#include "Define.h"
// ------------------------------------ //
#include "cef_v8.h"
#include "cef_values.h"

namespace Leviathan {

class NamedVars;

// There are some good conversion functions given here (and these approaches are taken from
// there): https://www.magpcss.org/ceforum/viewtopic.php?f=6&t=11104

//! \brief Converts a JS array to list value
//! \note source->IsArray() must be true
DLLEXPORT void JSArrayToList(
    const CefRefPtr<CefV8Value>& source, const CefRefPtr<CefListValue>& target);

//! \brief Takes a JS object and makes it into a dictionary for sending between processes
//! \note source->IsObject() must be true
DLLEXPORT void JSObjectToDictionary(
    const CefRefPtr<CefV8Value>& source, const CefRefPtr<CefDictionaryValue>& target);

//! \brief Converts CEF dictionary to NamedVars
DLLEXPORT void CEFDictionaryToNamedVars(
    const CefRefPtr<CefDictionaryValue>& source, NamedVars& destination);

} // namespace Leviathan
