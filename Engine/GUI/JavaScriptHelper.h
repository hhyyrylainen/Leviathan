// Leviathan Game Engine
// Copyright (c) 2012-2018 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "Common/DataStoring/NamedVars.h"

#include "include/cef_v8.h"

namespace Leviathan {

//! \brief Class that collects various JavaScript functions that would bloat other parts
class JavaScriptHelper {
public:
    //! \brief Converts a NamedVariableList to CefV8Value
    //! \pre The current thread must be inside V8 context (called from javascript or
    //! CefV8Context::Enter)
    DLLEXPORT static CefRefPtr<CefV8Value> ConvertNamedVariableListToJavaScriptValue(
        NamedVariableList* obj);

    //! \brief Converts a VariableBlock to CefV8Value
    //! \pre The current thread must be inside V8 context (called from javascript or
    //! CefV8Context::Enter)
    DLLEXPORT static CefRefPtr<CefV8Value> ConvertVariableBlockToJavaScriptValue(
        VariableBlock* block);

private:
    JavaScriptHelper();
    ~JavaScriptHelper();
};

} // namespace Leviathan
