#ifndef LEVIATHAN_CONSOLE
#define LEVIATHAN_CONSOLE
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "ScriptInterface.h"


namespace Leviathan{

	enum CONSOLECOMMANDRESULTSTATE {CONSOLECOMMANDRESULTSTATE_SUCCEEDED, CONSOLECOMMANDRESULTSTATE_FAILED, CONSOLECOMMANDRESULTSTATE_WAITINGFORMORE};
	enum CONSOLECOMMANDTYPE {CONSOLECOMMANDTYPE_NONE, CONSOLECOMMANDTYPE_ADDVAR, CONSOLECOMMANDTYPE_ADDFUNC, CONSOLECOMMANDTYPE_DELVAR, 
		CONSOLECOMMANDTYPE_DELFUNC, CONSOLECOMMANDTYPE_ERROR};

	// class used to execute script functions in the Console module //
	// note: console IS thread safe (at least should, work in progress)
	class ScriptConsole : public ThreadSafe{
	public:
		DLLEXPORT ScriptConsole();
		DLLEXPORT ~ScriptConsole();

		DLLEXPORT bool Init(ScriptInterface* MainScript);
		DLLEXPORT void Release();
		// this function takes direct wstring input from user and translates it to be used in one of the functions below //
		// console commands should be in format of ">[TYPE=""] [COMMAND]" eg. ">ADDVAR int newglobal = 25"
		// or "> for(int i = 0; i < 5; i++) GlobalFunc();" multiline commands are done by putting '\' to the end of each line
		DLLEXPORT int RunConsoleCommand(const wstring &commandstr);

		// calls script helper and runs this statement on the console module //
		DLLEXPORT bool ExecuteStringInstruction(string statement);
		DLLEXPORT bool AddVariableStringDefinition(string statement);
		DLLEXPORT bool DeleteVariableStringDefinition(string statement);
		DLLEXPORT bool AddFunctionStringDefinition(string statement);
		DLLEXPORT bool DeleteFunctionStringDefinition(string statement);

		// common functions that are in the style of the angel script sdk example console //
		DLLEXPORT void ListFunctions();
		DLLEXPORT void ListVariables();

	private:
		// function used to add prefix to console output //
		inline void ConsoleOutput(const wstring &text){
			Logger::Get()->Write(L"[CONSOLE] "+text);
		}

		// stored pointer to script interface //
		ScriptInterface* InterfaceInstance;

		// the script module that Console runs in //
		weak_ptr<ScriptModule> ConsoleModule;

		// variables about the current command //
		// multi line command stored part //
		wstring PendingCommand;

		int consoleemptyspam;

		// static map for special function definitions in command s//
		static map<wstring, CONSOLECOMMANDTYPE> CommandTypeDefinitions;
	};


}

#endif