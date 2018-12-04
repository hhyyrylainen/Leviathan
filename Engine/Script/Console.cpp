// ------------------------------------ //
#include "Script/Console.h"

#include "Application/Application.h"
#include "Iterators/StringIterator.h"
#include "ScriptModule.h"
#include "add_on/scripthelper/scripthelper.h"
using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT Leviathan::ScriptConsole::ScriptConsole() :
    InterfaceInstance(NULL), ConsoleModule(), consoleemptyspam(0)
{}

DLLEXPORT Leviathan::ScriptConsole::~ScriptConsole() {}

std::map<std::string, CONSOLECOMMANDTYPE> Leviathan::ScriptConsole::CommandTypeDefinitions = {
    {std::string("ADDVAR"), CONSOLECOMMANDTYPE_ADDVAR},
    {std::string("ADDFUNC"), CONSOLECOMMANDTYPE_ADDFUNC},
    {std::string("DELVAR"), CONSOLECOMMANDTYPE_DELVAR},
    {std::string("DELFUNC"), CONSOLECOMMANDTYPE_DELFUNC},
    {std::string("PRINTVAR"), CONSOLECOMMANDTYPE_PRINTVAR},
    {std::string("PRINTFUNC"), CONSOLECOMMANDTYPE_PRINTFUNC},
    {std::string("LISTVAR"), CONSOLECOMMANDTYPE_PRINTVAR},
    {std::string("LISTFUNC"), CONSOLECOMMANDTYPE_PRINTFUNC}};

// ------------------------------------ //
DLLEXPORT bool Leviathan::ScriptConsole::Init(ScriptExecutor* MainScript)
{

    // store pointer //
    InterfaceInstance = MainScript;

    // get a new module to be the console module //
    ConsoleModule = InterfaceInstance->CreateNewModule("ConsoleModule", "console");

    return true;
}

DLLEXPORT void Leviathan::ScriptConsole::Release()
{
    GUARD_LOCK();
    // set the module to release itself since it won't be used anymore //
    auto tmpptre = ConsoleModule.lock();

    if(tmpptre)
        tmpptre->DeleteThisModule();
}
// ------------------------------------ //
DLLEXPORT int Leviathan::ScriptConsole::RunConsoleCommand(std::string cmd)
{

    // Trim the end to make sure there aren't newline characters
    StringOperations::RemovePreceedingTrailingSpaces(cmd);

    GUARD_LOCK();


    // First thing to check is the user wanting help //
    if(cmd == "help") {

        ConsoleOutput("// ------------------ Help ------------------ //\n"
                      "\t> Console commands are a custom command followed by it's parameters\n"
                      "\t  or just plain AngelScript code. (Optionally starting with a '>')\n"
                      "\t> Running a custom command: \">[TYPE=\"\"] [COMMAND]\" eg. \n"
                      "\t  \">ADDVAR int newglobal = 25\"\n"
                      "\t You can view custom commands with the \"commands\" command.\n"
                      "\t> Running arbitrary commands:\n "
                      "\t  \"> for(int i = 0; i < 5; i++) GlobalFunc();\"\n"
                      "\t> Multiline commands are done by putting '\\' (a backwards slash) \n"
                      "\tto the end of each line.\n"
                      "\t> For example:\n"
                      "\t >ADDFUNC void MyFunc(int i){ Print(\"Val is: \"+i); }\n"
                      "\t(int i = 0; i < 10; i++){ MyFunc(i); }\n"
                      "\t> Would output \"Val is: 0 Val is: 1 ...\"");
        return CONSOLECOMMANDRESULTSTATE_SUCCEEDED;

    } else if(cmd == "commands") {
        // List custom commands //
        ConsoleOutput("// ------------------ Custom commands ------------------ //\n"
                      "Available custom commands are:\n");

        std::string messagecommand = "";
        bool first = true;

        for(auto iter = CommandTypeDefinitions.begin(); iter != CommandTypeDefinitions.end();
            ++iter) {
            // Add it's name //
            if(!first) {
                messagecommand += ", ";
            }

            messagecommand += iter->first;

            first = false;
        }

        ConsoleOutput("\t> " + messagecommand);
        return CONSOLECOMMANDRESULTSTATE_SUCCEEDED;

    } else if(cmd == "exit" || cmd == "quit" || cmd == "q") {

        ConsoleOutput("Marking the program as closing");
        Leviathan::LeviathanApplication::Get()->MarkAsClosing();
        return CONSOLECOMMANDRESULTSTATE_SUCCEEDED;
    }

    // first check if ">" is first character, we can easily reject command if it is missing //
    if(cmd.size() < 1) {
        // invalid format //
        ConsoleOutput("Invalid command format, empty command");
        consoleemptyspam++;
        if(consoleemptyspam > 5) {
            // \todo tell user how to close console //
            ConsoleOutput("You seem to be spamming empty lines, maybe you'd like to close "
                          "the console? \"quit\" \nor \"help\" might help you on your quest.");
        }

        return CONSOLECOMMANDRESULTSTATE_FAILED;
    }

    StringIterator itr(cmd);

    if(itr.GetCharacter() == '>') {
        // Skip first character since it is now handled //
        itr.MoveToNext();
    }

    // get the console main command type //
    auto ccmd = itr.GetNextCharacterSequence<std::string>(
        UNNORMALCHARACTER_TYPE_LOWCODES | UNNORMALCHARACTER_TYPE_WHITESPACE);

    // check if the length is too long or too short to actually be any specific command //
    CONSOLECOMMANDTYPE commandtype = CONSOLECOMMANDTYPE_NONE;
    if(ccmd && (ccmd->size() > 0)) {
        // Check for types //
        auto matchpos = CommandTypeDefinitions.find(*ccmd);

        if(matchpos == CommandTypeDefinitions.end()) {
            // Not found //
            commandtype = CONSOLECOMMANDTYPE_NONE;
        } else {
            // Set matching type //
            commandtype = matchpos->second;
        }

    } else {

        commandtype = CONSOLECOMMANDTYPE_ERROR;
    }

    auto restofcommand = itr.GetUntilEnd<std::string>();

    if((!restofcommand || restofcommand->empty()) && commandtype == CONSOLECOMMANDTYPE_NONE) {

        restofcommand.swap(ccmd);
    }

    // Switch on type and handle rest //
    switch(commandtype) {
    case CONSOLECOMMANDTYPE_NONE: {
        // We just need to check if this is multiple lines command //
        if(restofcommand->back() == '\\') {

            // Multi line command //
            if(ccmd) {

                PendingCommand +=
                    (*ccmd) + (restofcommand->substr(0, restofcommand->size() - 1)) + "\n";
            } else {

                PendingCommand += restofcommand->substr(0, restofcommand->size() - 1) + "\n";
            }

            // waiting for more //
            return CONSOLECOMMANDRESULTSTATE_WAITINGFORMORE;

        } else {

            // run command (and possibly previous multi line parts) //
            if(!ExecuteStringInstruction(
                   guard, PendingCommand.size() != 0 ?
                              PendingCommand + ((ccmd ? (*ccmd) : "")) + (*restofcommand) :
                              ((ccmd ? (*ccmd) : "")) + (*restofcommand))) {
                // Clear the pending command //
                PendingCommand.clear();
                // The command execution has failed... //
                return CONSOLECOMMANDRESULTSTATE_FAILED;
            }
            // clear pending command //
            PendingCommand.clear();
            // succeeded //
            return CONSOLECOMMANDRESULTSTATE_SUCCEEDED;
        }
    } break;
    case CONSOLECOMMANDTYPE_ADDVAR: {
        return AddVariableStringDefinition(guard, *restofcommand) ?
                   CONSOLECOMMANDRESULTSTATE_SUCCEEDED :
                   CONSOLECOMMANDRESULTSTATE_FAILED;
    } break;
    case CONSOLECOMMANDTYPE_ADDFUNC: {
        return AddFunctionStringDefinition(guard, *restofcommand) ?
                   CONSOLECOMMANDRESULTSTATE_SUCCEEDED :
                   CONSOLECOMMANDRESULTSTATE_FAILED;
    } break;
    case CONSOLECOMMANDTYPE_DELVAR: {
        return DeleteVariableStringDefinition(guard, *restofcommand) ?
                   CONSOLECOMMANDRESULTSTATE_SUCCEEDED :
                   CONSOLECOMMANDRESULTSTATE_FAILED;
    } break;
    case CONSOLECOMMANDTYPE_DELFUNC: {
        return DeleteFunctionStringDefinition(guard, *restofcommand) ?
                   CONSOLECOMMANDRESULTSTATE_SUCCEEDED :
                   CONSOLECOMMANDRESULTSTATE_FAILED;
    } break;
    case CONSOLECOMMANDTYPE_PRINTFUNC: {
        ListFunctions(guard);
    } break;
    case CONSOLECOMMANDTYPE_PRINTVAR: {
        ListVariables(guard);
    } break;
    default: {
        ConsoleOutput("Invalid command type, if you don't know what a command type is you"
                      "probably should add space after > \n"
                      "like: \"> yourstuffhere();\" OR just don't type the '>' \n"
                      "and everything should be fine.");
    }
    }
    // commands will return their codes if they succeed //
    return CONSOLECOMMANDRESULTSTATE_FAILED;
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::ScriptConsole::ExecuteStringInstruction(
    Lock& guard, const std::string& statement)
{
    std::unique_ptr<asIScriptContext, std::function<void(asIScriptContext*)>> context(
        InterfaceInstance->GetASEngine()->RequestContext(), [this](asIScriptContext* context) {
            InterfaceInstance->GetASEngine()->ReturnContext(context);
        });

    // Use ScriptHelper class to execute this statement in the module //
    int result = ExecuteString(InterfaceInstance->GetASEngine(), statement.c_str(),
        ConsoleModule.lock()->GetModule(), context.get());
    if(result < 0) {

        LOG_WRITE("Error in: " + statement);
        ConsoleOutput(
            "Invalid command syntax, type 'help' or refer to the AngelScript manual");
        return false;

    } else if(result == asEXECUTION_EXCEPTION) {

        // This is duplicated from ScriptExecutor. Should probably
        // refactor to merge this reporting to some function that
        // takes a report source

        InterfaceInstance->PrintExceptionInfo(context.get(), LogOutput);

        ConsoleOutput("Command caused an exception, more info is in the log, \n"
                      "depending on the exception it may or may not have been your command, \n"
                      "but rather a bug in someone else's code...");

        return false;
    }

    // Couldn't fail that badly //
    return true;
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::ScriptConsole::AddVariableStringDefinition(
    Lock& guard, std::string statement)
{
    // adds a variable using the method in the Console example of AngelScript SDK //

    int result = ConsoleModule.lock()->GetModule()->CompileGlobalVar(
        "ConsoleAddVar", statement.c_str(), 0);
    if(result < 0) {

        ConsoleOutput("Failed to add a new variable, log might have some more info");
        return false;
    }

    return true;
}

DLLEXPORT bool Leviathan::ScriptConsole::DeleteVariableStringDefinition(
    Lock& guard, const std::string& statement)
{
    // deletes a variable using the method in the Console example of AngelScript SDK //
    // get the variable by name //
    asIScriptModule* mod = ConsoleModule.lock()->GetModule();

    int index = mod->GetGlobalVarIndexByName(statement.c_str());
    if(index >= 0) {

        mod->RemoveGlobalVar(index);
        ConsoleOutput("Variable removed");
        return true;
    }
    ConsoleOutput("Variable not found");
    return false;
}

DLLEXPORT bool Leviathan::ScriptConsole::AddFunctionStringDefinition(
    Lock& guard, const std::string& statement)
{
    // adds a function using the method in the Console example of AngelScript SDK //
    bool result = false;

    asIScriptModule* mod = ConsoleModule.lock()->GetModule();


    asIScriptFunction* func = 0;
    int r = mod->CompileFunction(
        "ConsoleAddFunc", statement.c_str(), 0, asCOMP_ADD_TO_MODULE, &func);
    if(r < 0) {

        ConsoleOutput("Failed to add the function");
        result = false;
    } else {

        result = true;

        // we could disallow same function name with different arguments //
        // if(mod->GetFunctionByName(func->GetName()) == 0){

        //    mod->RemoveFunction(func);
        //    ConsoleOutput("Function with that name already exists");
        //    result = false;
        //}
    }

    // We must release the function object //
    if(func)
        func->Release();
    if(result)
        ConsoleOutput("Function added");
    return result;
}

DLLEXPORT bool Leviathan::ScriptConsole::DeleteFunctionStringDefinition(
    Lock& guard, const std::string& statement)
{
    // deletes a function using the method in the Console example of AngelScript SDK //
    asIScriptModule* mod = ConsoleModule.lock()->GetModule();

    // try to find by name //
    asIScriptFunction* func = mod->GetFunctionByName(statement.c_str());
    if(func) {
        // found, remove it //
        mod->RemoveFunction(func);

        goto funcdeletesucceedendgarbagecollectlabel;
    }
    // we couldn't find it by name //

    // try to find by declaration //
    func = mod->GetFunctionByDecl(statement.c_str());
    if(func) {
        // found, remove it //
        mod->RemoveFunction(func);

        goto funcdeletesucceedendgarbagecollectlabel;
    }

    ConsoleOutput("Function not found, if you tried with just the name try full declaration \n"
                  "\"int func(int arg1, int arg2)\"");

    return false;

funcdeletesucceedendgarbagecollectlabel:

    ConsoleOutput("Function deleted");

    // Since functions can be recursive, we'll call the garbage
    // collector to make sure the object is really freed
    // \todo make engine garbage collect stop all running scripts //
    Logger::Get()->Warning("Console: doing garbage cleanup, scripts might be running...");
    InterfaceInstance->GetASEngine()->GarbageCollect();

    return true;
}
// ------------------------------------ //
DLLEXPORT void Leviathan::ScriptConsole::ListFunctions(Lock& guard)
{

    // list global functions //
    Logger::Get()->Info("Global functions: ");

    // get pointer to engine //
    asIScriptEngine* engine = InterfaceInstance->GetASEngine();

    for(asUINT n = 0; n < engine->GetGlobalFunctionCount(); n++) {
        // get function pointer //
        asIScriptFunction* func = engine->GetGlobalFunctionByIndex(n);

        // Skip the functions that start with _ as these are not meant to be called explicitly
        // by the user
        if(func->GetName()[0] != '_')
            Logger::Get()->Write(std::string("\t> ") + func->GetDeclaration());
    }

    // list consoles' global variables //
    Logger::Get()->Info("Console instance functions: ");

    // List the user functions in the module
    asIScriptModule* mod = ConsoleModule.lock()->GetModule();

    for(asUINT n = 0; n < mod->GetFunctionCount(); n++) {
        // get function //
        asIScriptFunction* func = mod->GetFunctionByIndex(n);

        // Print the function //
        Logger::Get()->Write(std::string("\t> ") + func->GetDeclaration());
    }
}

DLLEXPORT void Leviathan::ScriptConsole::ListVariables(Lock& guard)
{

    // list global variables //
    Logger::Get()->Info("Global script variables: ");

    // get pointer to engine //
    asIScriptEngine* engine = InterfaceInstance->GetASEngine();

    for(asUINT n = 0; n < engine->GetGlobalPropertyCount(); n++) {
        // get info about variable //
        const char* name;
        int vartypeid;
        bool conststate;
        engine->GetGlobalPropertyByIndex(n, &name, 0, &vartypeid, &conststate);
        // construct info string //
        std::string decl(conststate ? "const " : "");
        decl += engine->GetTypeDeclaration(vartypeid);
        decl += " ";
        decl += name;

        Logger::Get()->Write("\t> " + decl);
    }

    // list consoles' global variables //
    Logger::Get()->Info("Console instance variables: ");

    // List the user variables in the module
    asIScriptModule* mod = ConsoleModule.lock()->GetModule();

    for(asUINT n = 0; n < mod->GetGlobalVarCount(); n++) {
        // print //
        Logger::Get()->Write(std::string("\t#> ") + mod->GetGlobalVarDeclaration(n));
    }
}

// ------------------------------------ //
// ConsoleLogger

void ConsoleLogger::Write(const std::string& text)
{

    Logger::Get()->Write("[CONSOLE]" + text);
}

void ConsoleLogger::WriteLine(const std::string& text)
{

    Logger::Get()->WriteLine("[CONSOLE]" + text);
}

void ConsoleLogger::Info(const std::string& text)
{

    Logger::Get()->Info("[CONSOLE]" + text);
}

void ConsoleLogger::Warning(const std::string& text)
{

    Logger::Get()->Warning("[CONSOLE]" + text);
}

void ConsoleLogger::Error(const std::string& text)
{

    Logger::Get()->Error("[CONSOLE]" + text);
}

void ConsoleLogger::Fatal(const std::string& text)
{

    Logger::Get()->Fatal("[CONSOLE]" + text);
}
