// Leviathan Game Engine
// Copyright (c) 2012-2017 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "Common/DataStoring/DataBlock.h"

#include "angelscript.h"

namespace Leviathan{

enum SCRIPT_RUNTYPE{SCRIPT_RUNTYPE_BREAKONERROR, SCRIPT_RUNTYPE_TRYTOCONTINUE};

class ScriptRunningSetup{
public:
    DLLEXPORT ScriptRunningSetup();
    DLLEXPORT ~ScriptRunningSetup();

    // named constructor idiom //
    DLLEXPORT inline ScriptRunningSetup& SetEntrypoint(const std::string &epoint){
        // set //
        Entryfunction = epoint;
        return *this;
    }

    // This is in the script executor
    // //! \param func Pointer to the script function. The caller must
    // //! keep a reference alive to this function as long as this run
    // //! setup is alive
    // DLLEXPORT inline ScriptRunningSetup& SetEntrypoint(asIScriptFunction* func){

    //     PreFetchedFunction = func;
    //     return *this;
    // }
        
    DLLEXPORT inline ScriptRunningSetup& SetArguments(
        std::vector<std::shared_ptr<NamedVariableBlock>> &args)
    {
        // set //
        Parameters = args;
        return *this;
    }
        
    DLLEXPORT inline ScriptRunningSetup& SetUseFullDeclaration(const bool &state){
        // set //
        FullDeclaration = state;
        return *this;
    }
        
    DLLEXPORT inline ScriptRunningSetup& SetPrintErrors(const bool &state){
            
        PrintErrors = state;
        return *this;
    }
		

    // variables //
    std::vector<std::shared_ptr<NamedVariableBlock>> Parameters;

    bool PrintErrors;
    bool FullDeclaration;
    bool ErrorOnNonExistingFunction;
    SCRIPT_RUNTYPE RunType;


    bool ScriptExisted;

    std::string Entryfunction;
};

}

