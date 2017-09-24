#pragma once
// ------------------------------------ //
#include "Define.h"
// ------------------------------------ //
#include "ScriptExecutor.h"
#include "ErrorReporter.h"


namespace Leviathan{

enum CONSOLECOMMANDRESULTSTATE {
    CONSOLECOMMANDRESULTSTATE_SUCCEEDED,
    CONSOLECOMMANDRESULTSTATE_FAILED,
    CONSOLECOMMANDRESULTSTATE_WAITINGFORMORE
};

enum CONSOLECOMMANDTYPE {
    CONSOLECOMMANDTYPE_NONE,
    CONSOLECOMMANDTYPE_ADDVAR,
    CONSOLECOMMANDTYPE_ADDFUNC,
    CONSOLECOMMANDTYPE_DELVAR, 
    CONSOLECOMMANDTYPE_DELFUNC,
    CONSOLECOMMANDTYPE_PRINTVAR,
    CONSOLECOMMANDTYPE_PRINTFUNC,
    CONSOLECOMMANDTYPE_ERROR
};

//! \brief Logger for console output to the standard logger
class ConsoleLogger : public LErrorReporter{
public:
    virtual void Write(const std::string &text) override;

    virtual void WriteLine(const std::string &text) override;

    virtual void Info(const std::string &text) override;

    virtual void Warning(const std::string &text) override;

    virtual void Error(const std::string &text) override;

    virtual void Fatal(const std::string &text) override;
};

//! class used to execute script functions in the Console module
//! \note console IS thread safe (at least should, work in progress
class ScriptConsole : public ThreadSafe{
public:
    DLLEXPORT ScriptConsole();
    DLLEXPORT ~ScriptConsole();

    DLLEXPORT bool Init(ScriptExecutor* MainScript);
    DLLEXPORT void Release();
    
    // this function takes direct std::string input from user and translates it to be used in
    // one of the functions below
    // console commands should be in format of ">[TYPE=""] [COMMAND]" eg.
    // ">ADDVAR int newglobal = 25"
    // or "> for(int i = 0; i < 5; i++) GlobalFunc();" multiline commands are done by
    // putting '\' to the end of each line
    DLLEXPORT int RunConsoleCommand(std::string cmd);

    //! calls script helper and runs this statement on the console module
    DLLEXPORT bool ExecuteStringInstruction(Lock &guard, const std::string &statement);

    DLLEXPORT inline bool ExecuteStringInstruction(const std::string &statement){

        GUARD_LOCK();
        return ExecuteStringInstruction(guard, statement);
    }
        
    DLLEXPORT bool AddVariableStringDefinition(Lock &guard, std::string statement);
    DLLEXPORT bool DeleteVariableStringDefinition(Lock &guard, const std::string &statement);
    DLLEXPORT bool AddFunctionStringDefinition(Lock &guard, const std::string &statement);
    DLLEXPORT bool DeleteFunctionStringDefinition(Lock &guard, const std::string &statement);

    // common functions that are in the style of the angel script sdk example console //
    DLLEXPORT void ListFunctions(Lock &guard);
    DLLEXPORT void ListVariables(Lock &guard);

private:
    // function used to add prefix to console output //
    inline void ConsoleOutput(const std::string &text){
        LogOutput.Write("[CONSOLE] " + text);
    }

    ConsoleLogger LogOutput;

    // stored pointer to script interface //
    ScriptExecutor* InterfaceInstance;

    // the script module that Console runs in //
    std::weak_ptr<ScriptModule> ConsoleModule;

    // variables about the current command //
    // multi line command stored part //
    std::string PendingCommand;

    int consoleemptyspam;

    // static map for special function definitions in command s//
    static std::map<std::string, CONSOLECOMMANDTYPE> CommandTypeDefinitions;
};


}

