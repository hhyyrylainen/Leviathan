// Leviathan Game Engine
// Copyright (c) 2012-2017 Henri Hyyryläinen
#pragma once
// ------------------------------------ //
#include "BindHelpers.h"
#include "Logger.h"

namespace Leviathan{

template<class WorldType>
    bool BindStandardWorldMethods(asIScriptEngine* engine, const char* classname){

    #include "Generated/StandardWorldBindings.h"
    
    return true;
}
}



