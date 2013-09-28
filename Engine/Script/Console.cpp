#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_CONSOLE
#include "Script/Console.h"
#endif
#include "Utility\Iterators\WstringIterator.h"
#include "boost/assign.hpp"
#include "add_on/scripthelper/scripthelper.h"
using namespace Leviathan;
// ------------------------------------ //

DLLEXPORT Leviathan::ScriptConsole::ScriptConsole() : InterfaceInstance(NULL), ConsoleModule(){
	consoleemptyspam = 0;
}

DLLEXPORT Leviathan::ScriptConsole::~ScriptConsole(){

}


map<wstring, CONSOLECOMMANDTYPE> Leviathan::ScriptConsole::CommandTypeDefinitions = boost::assign::map_list_of
	(wstring(L"NONE"), CONSOLECOMMANDTYPE_NONE) (wstring(L"ADDVAR"), CONSOLECOMMANDTYPE_ADDVAR) (wstring(L"ADDFUNC"), CONSOLECOMMANDTYPE_ADDFUNC) 
	(wstring(L"DELVAR"), CONSOLECOMMANDTYPE_DELVAR) (wstring(L"DELFUNC"), CONSOLECOMMANDTYPE_DELFUNC);

// ------------------------------------ //
DLLEXPORT bool Leviathan::ScriptConsole::Init(ScriptInterface* MainScript){
	// store pointer //
	InterfaceInstance = MainScript;

	// get a new module to be the console module //
	ConsoleModule = InterfaceInstance->GetExecutor()->CreateNewModule(L"ConsoleModule", "console");

	return true;
}

DLLEXPORT void Leviathan::ScriptConsole::Release(){
	// set the module to release itself since it won't be used anymore //
	shared_ptr<ScriptModule> tmpptre(ConsoleModule.lock());
	if(tmpptre.get() != NULL){
		// set the module to unallocate itself //
		tmpptre->DeleteThisModule();
	}
}
// ------------------------------------ //
DLLEXPORT int Leviathan::ScriptConsole::RunConsoleCommand(const wstring &commandstr){
	// we use an iterator for going through the command //

	// first check if ">" is first character, we can easily reject command if it is missing //
	if(commandstr.size() < 1 || commandstr[0] != L'>'){
		// invalid format //
		ConsoleOutput(L"Invalid command format, missing starting '>' or empty command");
		consoleemptyspam++;
		if(consoleemptyspam > 5){
			// TODO: tell user how to close console //
			ConsoleOutput(L"You seem to be spamming empty lines, maybe you'd like to close console? TODO: actually write help so that user could actually close the console");
		}
		return CONSOLECOMMANDRESULTSTATE_FAILED;
	}

	WstringIterator itr(commandstr);

	// skip first character since it is checked //
	itr.MoveToNext();

	// get the console main command type //
	unique_ptr<wstring> ccmd = itr.GetNextCharacterSequence(UNNORMALCHARACTER_TYPE_LOWCODES | UNNORMALCHARACTER_TYPE_WHITESPACE);

	// check if the length is too long or too short to actually be any specific command //
	CONSOLECOMMANDTYPE commandtype = CONSOLECOMMANDTYPE_NONE;
	if(ccmd->size() > 0 && ccmd->size() < 14){
		// check for types //
		auto matchpos = CommandTypeDefinitions.find(*ccmd);
		if(matchpos == CommandTypeDefinitions.end()){
			// not found //
			commandtype = CONSOLECOMMANDTYPE_ERROR;
		} else {
			// set matching type //
			commandtype = matchpos->second;
		}
	}

	unique_ptr<wstring> restofcommand = itr.GetUntilEnd();

	// switch on type and handle rest //
	switch (commandtype){
	case CONSOLECOMMANDTYPE_NONE:
		{
			// we just need to check if this is multiple lines command //
			if(restofcommand->back() == L'\\'){
				// multi line command //

				PendingCommand += (*ccmd)+(restofcommand->substr(0, restofcommand->size()-1))+L"\n";
				// waiting for more //
				return CONSOLECOMMANDRESULTSTATE_WAITINGFORMORE;

			} else {
				// run command (and possibly previous multi line parts) //

				if(!ExecuteStringInstruction(Convert::WstringToString(PendingCommand.size() != 0 ? PendingCommand+(*ccmd)+(*restofcommand): 
					(*ccmd)+(*restofcommand))))
				{
					// clear pending command //
					PendingCommand.clear();
					// failed //
					return CONSOLECOMMANDRESULTSTATE_FAILED;
				}
				// clear pending command //
				PendingCommand.clear();
				// succeeded //
				return CONSOLECOMMANDRESULTSTATE_SUCCEEDED;
			}
		}
	break;
	case CONSOLECOMMANDTYPE_ADDVAR:
		{
			return AddVariableStringDefinition(Convert::WstringToString(*restofcommand)) ? CONSOLECOMMANDRESULTSTATE_SUCCEEDED: 
				CONSOLECOMMANDRESULTSTATE_FAILED;
		}
	break;
	case CONSOLECOMMANDTYPE_ADDFUNC:
		{
			return AddFunctionStringDefinition(Convert::WstringToString(*restofcommand)) ? CONSOLECOMMANDRESULTSTATE_SUCCEEDED: 
				CONSOLECOMMANDRESULTSTATE_FAILED;
		}
	break;
	case CONSOLECOMMANDTYPE_DELVAR:
		{
			return DeleteVariableStringDefinition(Convert::WstringToString(*restofcommand)) ? CONSOLECOMMANDRESULTSTATE_SUCCEEDED: 
				CONSOLECOMMANDRESULTSTATE_FAILED;
		}
	break;
	case CONSOLECOMMANDTYPE_DELFUNC:
		{
			return DeleteFunctionStringDefinition(Convert::WstringToString(*restofcommand)) ? CONSOLECOMMANDRESULTSTATE_SUCCEEDED: 
				CONSOLECOMMANDRESULTSTATE_FAILED;
		}
		break;
	default:
		{
			ConsoleOutput(L"Invalid command type, if you don't know what a command type is you probably should add space after > like: \"> yourstuffhere();\"");
		}
	}
	// commands will return their codes if they succeed //
	return CONSOLECOMMANDRESULTSTATE_FAILED;
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::ScriptConsole::ExecuteStringInstruction(string statement){
	// use ScriptHelper class to execute this statement in the module //
	int result = ExecuteString(InterfaceInstance->GetExecutor()->GetASEngine(), statement.c_str(), ConsoleModule.lock()->GetModule());
	if(result < 0){

		ConsoleOutput(L"Invalid command syntax, refer to AngelScript manual for right syntax or whatever tutorial(s) you may have found");
		return false;

	} else if(result == asEXECUTION_EXCEPTION){

		ConsoleOutput(L"Command caused an exception, more info is in the log, depending on the exception it may or may not have been your command but"
			L"rather a bug in someone else's code");
		return false;
	}
	// couldn't fail that badly //
	return true;
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::ScriptConsole::AddVariableStringDefinition(string statement){
	// adds a variable using the method in the Console example of AngelScript SDK //

	// force semicolon to the end of the statement //
	if(statement.length() > 0 && statement.back() != ';')
		statement += ';';

	int result = ConsoleModule.lock()->GetModule()->CompileGlobalVar("ConsoleAddVar", statement.c_str(), 0);
	if(result < 0){

		ConsoleOutput(L"Failed to add a new variable, log might have some more info");
		return false;
	}

	return true;
}

DLLEXPORT bool Leviathan::ScriptConsole::DeleteVariableStringDefinition(string statement){
	// deletes a variable using the method in the Console example of AngelScript SDK //
	// get the variable by name //
	asIScriptModule* mod = ConsoleModule.lock()->GetModule();

	int index = mod->GetGlobalVarIndexByName(statement.c_str());
	if(index >= 0 ){

		mod->RemoveGlobalVar(index);
		ConsoleOutput(L"Variable removed");
		return true;
	}
	ConsoleOutput(L"Variable not found");
	return false;
}

DLLEXPORT bool Leviathan::ScriptConsole::AddFunctionStringDefinition(string statement){
	// adds a function using the method in the Console example of AngelScript SDK //
	bool result = false;

	asIScriptModule* mod = ConsoleModule.lock()->GetModule();


	asIScriptFunction* func = 0;
	int r = mod->CompileFunction("ConsoleAddFunc", statement.c_str(), 0, asCOMP_ADD_TO_MODULE, &func);
	if(r < 0){
		
		ConsoleOutput(L"Failed to add the function");
		result = false;
	} else {

		result = true;

		// we could disallow same function name with different arguments //
		//if(mod->GetFunctionByName(func->GetName()) == 0){

		//	mod->RemoveFunction(func);
		//	ConsoleOutput(L"Function with that name already exists");
		//	result = false;
		//}
	}

	// We must release the function object // (why?, I have no idea) //
	if(func)
		func->Release();
	if(result)
		ConsoleOutput(L"Function added");
	return result;
}

DLLEXPORT bool Leviathan::ScriptConsole::DeleteFunctionStringDefinition(string statement){
	// deletes a function using the method in the Console example of AngelScript SDK //
	asIScriptModule* mod = ConsoleModule.lock()->GetModule();

	// try to find by name //
	asIScriptFunction* func = mod->GetFunctionByName(statement.c_str());
	if(func){
		// found, remove it //
		mod->RemoveFunction(func);
		
		goto funcdeletesucceedendgarbagecollectlabel;
	}
	// we couldn't find it by name //
	
	// try to find by declaration //
	func = mod->GetFunctionByDecl(statement.c_str());
	if(func){
		// found, remove it //
		mod->RemoveFunction(func);

		goto funcdeletesucceedendgarbagecollectlabel;

	}

	ConsoleOutput(L"Function not found, if you tried with just the name try full declaration \"int func(int arg1, int arg2)\"");

	return false;

funcdeletesucceedendgarbagecollectlabel:

	ConsoleOutput(L"Function deleted");

	// Since functions can be recursive, we'll call the garbage
	// collector to make sure the object is really freed
	// TODO: make engine garbage collect stop all running scripts //
	InterfaceInstance->GetExecutor()->GetASEngine()->GarbageCollect();

	return true;
}
// ------------------------------------ //
DLLEXPORT void Leviathan::ScriptConsole::ListFunctions(){
	// list global functions //
	Logger::Get()->Info(L"Global functions: ");

	// get pointer to engine //
	asIScriptEngine* engine = InterfaceInstance->GetExecutor()->GetASEngine();

	for(asUINT n = 0; n < engine->GetGlobalFunctionCount(); n++){
		// get function pointer //
		asIScriptFunction* func = engine->GetGlobalFunctionByIndex(n);

		// Skip the functions that start with _ as these are not meant to be called explicitly by the user
		if(func->GetName()[0] != '_')
			Logger::Get()->Write(Convert::StringToWstring(func->GetDeclaration()));
	}
	// list consoles' global variables //
	Logger::Get()->Info(L"Console instance functions: ");

	// List the user functions in the module
	asIScriptModule* mod = ConsoleModule.lock()->GetModule();

	for(asUINT n = 0; n < mod->GetFunctionCount(); n++ ){
		// get function //
		asIScriptFunction* func = mod->GetFunctionByIndex(n);
		// print the function //
		Logger::Get()->Write(Convert::StringToWstring(func->GetDeclaration()));
	}

}

DLLEXPORT void Leviathan::ScriptConsole::ListVariables(){

	// list global variables //
	Logger::Get()->Info(L"Global script variables: ");

	// get pointer to engine //
	asIScriptEngine* engine = InterfaceInstance->GetExecutor()->GetASEngine();

	for(asUINT n = 0; n < engine->GetGlobalPropertyCount(); n++){
		// get info about variable //
		const char* name;
		int vartypeid;
		bool conststate;
		engine->GetGlobalPropertyByIndex(n, &name, 0, &vartypeid, &conststate);
		// construct info string //
		string decl(conststate ? "const " : "");
		decl += engine->GetTypeDeclaration(vartypeid);
		decl += " ";
		decl += name;
		
		Logger::Get()->Write(L"> "+Convert::StringToWstring(decl));
	}
	// list consoles' global variables //
	Logger::Get()->Info(L"Console instance variables: ");

	// List the user variables in the module
	asIScriptModule* mod = ConsoleModule.lock()->GetModule();

	for(asUINT n = 0; n < mod->GetGlobalVarCount(); n++ ){
		// print //
		Logger::Get()->Write(L"# "+Convert::StringToWstring(mod->GetGlobalVarDeclaration(n)));
	}
}


