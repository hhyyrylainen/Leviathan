#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_SCRIPT_EXECUTOR
#include "ScriptExecutor.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
#include "AngelScriptCommon.h"
#include <add_on\scriptstdstring\scriptstdstring.h>
#include <add_on\scriptarray\scriptarray.h>

// headers that contains bindable functions //
#include "GuiScriptInterface.h"

ScriptExecutor::ScriptExecutor(){
	engine = NULL;
}
ScriptExecutor::~ScriptExecutor(){

}



const wstring Prefixes = L" for while do ";


// ------------------------------------ //
ScriptModule::ScriptModule(){
	ID = -1;
	ModuleID = -1;
	Name = L"ERROR";
}
ScriptModule::~ScriptModule(){

}
ScriptModule::ScriptModule(wstring name, int id){
	ID = id;
	LatestAssigned++;
	ModuleID = LatestAssigned;
	Name = name;
}

int ScriptModule::LatestAssigned = -1;
int Val_NOVALVAL_FORscriptptr = VAL_NOUPDATE;

// ------------------------------------ //
bool ScriptExecutor::Init(){
	// initialize angelscript //
	engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	if(engine == NULL){

		Logger::Get()->Error(L"ScriptExecutor: Init: cannot create angelscript engine", false);
		return false;
	}
	// set callback to error report function //
	engine->SetMessageCallback(asFUNCTION(ScriptMessageCallback), 0, asCALL_CDECL);

	// register script string type //
	RegisterStdString(engine);


	// register global functions and classes //
	if(engine->RegisterGlobalFunction("void Print(string message, bool save = true)", asFUNCTION(Logger::Print), asCALL_CDECL) < 0){
		// error abort //
		Logger::Get()->Error(L"ScriptExecutor: Init: AngelScript: registerglobal failed in main ex file on line"+Convert::IntToWstring(__LINE__), false);
		return false;
	}
	//VAL_NOUPDATE
	// global enums/values //
	if(engine->RegisterGlobalProperty("const int VAL_NOUPDATE", &Val_NOVALVAL_FORscriptptr) < 0){
		// error abort //
		Logger::Get()->Error(L"ScriptExecutor: Init: AngelScript: registerglobal failed  in main ex file on line"+Convert::IntToWstring(__LINE__), false);
		return false;
	}

	//if(engine->RegisterGlobalFunction("bool Gui_SetObjectText(int id, string toset, bool doupdate = true)", asFUNCTION(Gui_SetObjectText), asCALL_CDECL) < 0){
	//	// error abort //
	//	Logger::Get()->Error(L"ScriptExecutor: Init: AngelScript: registerglobal failed on line"+Convert::IntToWstring(__LINE__), false);
	//	return false;
	//}
	// include various headers that bind functions and classes //
#include "GuiScriptBind.h"

	return true;
}
bool ScriptExecutor::Release(){
	// release angelscript //
	engine->Release();

	while(Modules.size() != 0){
		SAFE_DELETE(Modules[0]);
		Modules.erase(Modules.begin());
	}
	return true;
}
// ------------------------------------ //
shared_ptr<ScriptArguement> ScriptExecutor::RunScript(ScriptScript* script, vector<ScriptCaller*> callers, vector<shared_ptr<ScriptVariableHolder>> gvalues, vector<ScriptNamedArguement*> parameters, bool printerrors, wstring entrance,
											bool ErrorIfdoesnt, bool fulldecl, int runtype){

	SetScript(script);
	SetCallers(callers);
	SetGlobalValues(gvalues);
	SetParameters(parameters);

	SetBehavior(printerrors, runtype);
	// run //
	shared_ptr<ScriptArguement> returnarg = RunSetUp(entrance, fulldecl, ErrorIfdoesnt);

	// clean up//
	Clear();
	return returnarg;
}
shared_ptr<ScriptArguement> ScriptExecutor::RunSetUp(wstring entrance, bool fulldecl, bool ErrorIfdoesnt){
	// check data validness //
	if(Callbacks.size() == 0){

		Logger::Get()->Error(L"ScriptExecutor: RunSetUp: no registered callers");
		return shared_ptr<ScriptArguement>(new ScriptArguement(NULL, DATABLOCK_TYPE_INT, true));
	}
	//if(Globals.size() == 0){

	//	Logger::Get()->Error(L"ScriptExecutor: RunSetUp: no registered global variables");
	//	return ScriptArguement(NULL, DATABLOCK_TYPE_INT, true);
	//}
	ScriptsValues = new ScriptVariableHolder();
	if(RunningScripts == NULL){
		Logger::Get()->Error(L"ScriptExecutor: RunSetUp: no script to run!");
		return shared_ptr<ScriptArguement>(new ScriptArguement(NULL, DATABLOCK_TYPE_INT, true));
	}

	//// find entry script //
	//int startindex = -1;
	//for(int i = 0; i < RunningScripts.size(); i++){
	//	if(RunningScripts[i]->Name== entrance){
	//		startindex = i;
	//	}
	//}
	//ARR_INDEX_CHECKINV(startindex, RunningScripts.size()){

	//	Logger::Get()->Error(L"ScriptExecutor: RunSetUp: No entry point found "+entrance);
	//	return new ScriptArguement(NULL, DATABLOCK_TYPE_INT, true);
	//}
	

	

	// run script //
	// return return value //
	try{
		return RunScript(entrance, fulldecl, ErrorIfdoesnt);
	}
	catch (ScriptException &e){
		if(PrintErrors){
			Logger::Get()->Error(L"ScriptExecutor:  while executing : "+RunningScripts->Name+L" exception: "+Convert::IntToWstring(e.ErrorCode)+L" "+e.Message+L" action "+Convert::IntToWstring(e.ActionValue)+L" from "+e.Source, true);

		}
		Errors.push_back(new ScriptException(e));
		return shared_ptr<ScriptArguement>(new ScriptArguement(new IntBlock(80000800), DATABLOCK_TYPE_INT, true));
	}
	catch (exception &e){
		
		Logger::Get()->Error(L"ScriptExecutor:  exception : "+Convert::StringToWstringNonRef(e.what()), true);

		return shared_ptr<ScriptArguement>(new ScriptArguement(new IntBlock(80000800), DATABLOCK_TYPE_INT, true));
	}
	catch (string &e){
		
		Logger::Get()->Error(L"ScriptExecutor:  exception : "+Convert::StringToWstring(e), true);

		return shared_ptr<ScriptArguement>(new ScriptArguement(new IntBlock(80000800), DATABLOCK_TYPE_INT, true));
	}
	catch (int e){
		
		Logger::Get()->Error(L"ScriptExecutor:  exception : "+Convert::ToWstring(e), true);

		return shared_ptr<ScriptArguement>(new ScriptArguement(new IntBlock(80000800), DATABLOCK_TYPE_INT, true));
	}
	catch (...){
		
		Logger::Get()->Error(L"ScriptExecutor:  exception : UNKOWN EXCEPTION, MIGHT BE FATAL ERROR!", true);

		return shared_ptr<ScriptArguement>(new ScriptArguement(new IntBlock(80000800), DATABLOCK_TYPE_INT, true));
	}
}
shared_ptr<ScriptArguement> ScriptExecutor::RunScript(ScriptScript* script, ScriptCaller* caller, shared_ptr<ScriptVariableHolder> gvalues, ScriptNamedArguement* parameter, bool printerrors, wstring entrance, 
											bool ErrorIfdoesnt, bool fulldecl, int runtype){

	vector<ScriptCaller*> tempcv;
	vector<shared_ptr<ScriptVariableHolder>> tempgv;
	vector<ScriptNamedArguement*> temppa;


	tempcv.push_back(caller);
	temppa.push_back(parameter);
	if(gvalues != NULL)
		tempgv.push_back(gvalues);

	SetScript(script);
	SetCallers(tempcv);
	SetGlobalValues(tempgv);
	SetParameters(temppa);


	SetBehavior(printerrors, runtype);
	// run //
	shared_ptr<ScriptArguement> returnarg = RunSetUp(entrance, fulldecl, ErrorIfdoesnt);

	// clean up//
	Clear();
	return returnarg;
}
// ------------------------------------ //
void ScriptExecutor::SetScript(ScriptScript* script){
	RunningScripts = script;
}
void ScriptExecutor::SetCallers(vector<ScriptCaller*> callers){
	Callbacks = callers;
}
void ScriptExecutor::SetGlobalValues(vector<shared_ptr<ScriptVariableHolder>> gvalues){
	Globals = gvalues;
}
void ScriptExecutor::SetParameters(vector<ScriptNamedArguement*> parameters){
	Parameters = parameters;
}
void ScriptExecutor::SetBehavior(bool printerrors, int runtype){
	PrintErrors = printerrors;
	RunType = runtype;
}
void ScriptExecutor::Clear(){
	Callbacks.clear();
	Globals.clear();
	SAFE_DELETE(ScriptsValues);
	RunningScripts = NULL;

	//catch(ScriptException e){
	//	Logger::Get()->Error(L"ScriptCaller: calling function: "+name+L" exception: "+Convert::IntToWstring(e.ErrorCode)+L" "+e.Message+L" action "+Convert::IntToWstring(e.ActionValue)+L" from "+e.Source, true);
	//}
	while(Errors.size() != 0){
		Logger::Get()->Error(L"ScriptExecutor:  exception: "+Convert::IntToWstring(Errors[0]->ErrorCode)+L" "+Errors[0]->Message+L" action "+Convert::IntToWstring(Errors[0]->ActionValue)+L" from "+Errors[0]->Source, true);
		SAFE_DELETE(Errors[0]);
		Errors.erase(Errors.begin());
	}
}
// ------------------------------------ //
shared_ptr<ScriptArguement> ScriptExecutor::RunScript(wstring start, bool fulldecl, bool ErrorIfdoesnt){
	//ARR_INDEX_CHECKINV(index, RunningScripts.size()){

	//	Logger::Get()->Error(L"ScriptExecutor: RunScript No script found for index ", index, true);
	//	return new ScriptArguement(NULL, DATABLOCK_TYPE_INT, true);
	//}
	ScriptScript* script = RunningScripts;

	ScriptVariableHolder localvariables;

	
	if(script->Instructions.size() < 1){
		Logger::Get()->Error(L"ScriptExecutor: RunScript No instructions inside script name: "+script->Name, true);
		return shared_ptr<ScriptArguement>(new ScriptArguement(new IntBlock(80000800), DATABLOCK_TYPE_INT, true));
	}

	// load script //
	asIScriptModule* Module = LoadScript(script);
	if(NULL == Module){
		Logger::Get()->Error(L"ScriptExecutor: RunScript: Failed to compile script "+script->Name, true);
		return shared_ptr<ScriptArguement>(new ScriptArguement(new IntBlock(80000800), DATABLOCK_TYPE_INT, true));
	}

	// run script //
	asIScriptContext *ScriptContext = engine->CreateContext();
	if(NULL == ScriptContext){
		Logger::Get()->Error(L"ScriptExecutor: RunScript: Failed to create context for script: "+script->Name, true);
		SAFE_RELEASE(ScriptContext);
		return shared_ptr<ScriptArguement>(new ScriptArguement(new IntBlock(80000800), DATABLOCK_TYPE_INT, true));
	}

	// for now just run scripts: TODO: do script executorer //
	asIScriptFunction *func = NULL;
	string namey = Convert::WstringToString(start);
	if(!fulldecl){
		func = Module->GetFunctionByName(namey.c_str());
	} else {
		func = Module->GetFunctionByDecl(namey.c_str());
	}
	if(func == NULL){
		if(ErrorIfdoesnt){
			Logger::Get()->Error(L"ScriptExecutor: RunScript: Could not find starting function: "+start+L" in:"+script->Name, true);
			SAFE_RELEASE(ScriptContext);
			return shared_ptr<ScriptArguement>(new ScriptArguement(new IntBlock(80000800), DATABLOCK_TYPE_INT, true));
		} else {
			return shared_ptr<ScriptArguement>(new ScriptArguement(new IntBlock(80000802), DATABLOCK_TYPE_INT, true));
		}
	}

	if(ScriptContext->Prepare(func) < 0){
		Logger::Get()->Error(L"ScriptExecutor: RunScript: prepare context failed func: "+start+L" in:"+script->Name, true);
		SAFE_RELEASE(ScriptContext);
		return shared_ptr<ScriptArguement>(new ScriptArguement(new IntBlock(80000800), DATABLOCK_TYPE_INT, true));
	}

	// pass arguments // // figure out if arguments match function declaration //
	unsigned int parameterc = func->GetParamCount();
	wstring declaration = Convert::StringToWstringNonRef(func->GetDeclaration(false));

	// strip declaration //
	//wstring params = L"";
	vector<wstring> Params;
	bool found = false;
	bool ready = false;
	bool ignoring = false;
	for(unsigned int i = 0; i < declaration.size(); i++){
		if(found){
			if(declaration[i] == L')'){
				// ended //
				found = true;
				break;
			}
			if(!ready){
				Params.push_back(wstring());
				ready = true;
			}
			if(declaration[i] == L','){
				// another //
				ready = false;
				ignoring = false;
				continue;
			}
			if(declaration[i] == L' '){
				// skip //
				//ready = false;
				continue;
			}
			if(declaration[i] == L'='){
				// default parameter, start ignoring //
				//ready = false;
				ignoring = true;
				continue;
			}
			if(!ignoring)
				Params.back().push_back(declaration[i]);

			continue;
		}
		// check has declstarted //
		if(declaration[i] == L'(')
			found = true;
	}

	if(parameterc != Params.size()){
		// failed to successfully parse parameters //
		Logger::Get()->Error(L"ScriptExecutor: RunScript: PREpass parameters failed, could not get parameter types in func: "+start+L" named: "+script->Name+L" param count: "+Convert::IntToWstring(parameterc), Params.size(), true);
		return shared_ptr<ScriptArguement>(new ScriptArguement(new IntBlock(80000804), DATABLOCK_TYPE_INT, true));
	}

	// start passing parameters //
	for(unsigned int i = 0; i < parameterc; i++){
		if(i >= Parameters.size()) // no more parameters //
			break;
		// check do they match //
		bool l_error = false;

		int typeondecl = 0;

		typeondecl = Misc::WstringTypeNameCheck(Params[i]);
		if(typeondecl == 7){
			Logger::Get()->Error(L"ScriptExecutor: RunScript: pass parameters failed, invalid parameter types in func: "+start+L" named: "+script->Name+L" param typename: "+Params[i], typeondecl, true);
			return shared_ptr<ScriptArguement>(new ScriptArguement(new IntBlock(80000804), DATABLOCK_TYPE_INT, true));
		}

		// compare //
		if(!Misc::CompareDataBlockTypeToTHISNameCheck(Parameters[i]->Type, typeondecl)){
			// might not match //


			l_error = true;
		}

		if(l_error){
			Logger::Get()->Error(L"ScriptExecutor: RunScript: pass parameters failed in func: "+start+L" named: "+script->Name+L" param number: "+Convert::IntToWstring(i), i, true);
			return shared_ptr<ScriptArguement>(new ScriptArguement(new IntBlock(80000804), DATABLOCK_TYPE_INT, true));
		}
		// pass //
		switch(Parameters[i]->Type){
			case DATABLOCK_TYPE_INT:
				{
					ScriptContext->SetArgDWord(i, (int)*Parameters[i]);
				}
			break;
			case DATABLOCK_TYPE_FLOAT:
				{
					ScriptContext->SetArgFloat(i, (float)*Parameters[i]);
				}
			break;
			case DATABLOCK_TYPE_BOOL:
				{
					ScriptContext->SetArgByte(i, (bool)*Parameters[i]);
				}
			break;
			case DATABLOCK_TYPE_WSTRING:
				{
					// save as a string that can be retrieved //
					// not done
				}
			break;
			case DATABLOCK_TYPE_VOIDPTR:
				{
					// not done
				}
			break;
		}
	}

	//if(Parameters.size() > parameterc){

	//}
	//for(int i = 0; i < Parameters.size(); i++){
	//	switch(Parameters[i]->Type){
	//	case DATABLOCK_TYPE_INT:
	//		{
	//			ScriptContext->SetArgDWord(i, (int)*Parameters[i]);
	//		}
	//	break;
	//	case DATABLOCK_TYPE_FLOAT:
	//		{
	//			ScriptContext->SetArgFloat(i, (float)*Parameters[i]);
	//		}
	//	break;
	//	case DATABLOCK_TYPE_BOOL:
	//		{
	//			ScriptContext->SetArgByte(i, (bool)*Parameters[i]);
	//		}
	//	break;
	//	case DATABLOCK_TYPE_WSTRING:
	//		{
	//			// save as a string that can be retrieved //
	//			// not done
	//		}
	//	break;
	//	case DATABLOCK_TYPE_VOIDPTR:
	//		{
	//			// not done
	//		}
	//	break;
	//	}
	//}

	// run script //
	// could use timeout here //
	int retcode = ScriptContext->Execute();
	if(retcode != asEXECUTION_FINISHED){
		// something went wrong //

		// The execution didn't finish as we had planned. Determine why.
		if(retcode == asEXECUTION_ABORTED){
			// code took too long //
		} else if(retcode == asEXECUTION_EXCEPTION){
			// script caused an exception //
			asIScriptFunction *func = ScriptContext->GetExceptionFunction();

			Logger::Get()->Error(L"[SCRIPT] from: func: "+Convert::ToWstring(func->GetDeclaration())+L" modl: "+Convert::ToWstring(func->GetModuleName())+L" sect: "+Convert::ToWstring(func->GetScriptSectionName()), false);
			throw ScriptException(retcode, Convert::StringToWstringNonRef(ScriptContext->GetExceptionString()), script->Source+L" line: "+Convert::ToWstring(ScriptContext->GetExceptionLineNumber()));
		}
		SAFE_RELEASE(ScriptContext);
		return shared_ptr<ScriptArguement>(new ScriptArguement(new IntBlock(80000800), DATABLOCK_TYPE_INT, true));
	}
	// succesfully executed, try to fetch return value //
	// for now just return it as a float //
	shared_ptr<ScriptArguement> retrval = shared_ptr<ScriptArguement>(new ScriptArguement(new IntBlock(ScriptContext->GetReturnDWord()), DATABLOCK_TYPE_INT, true));
	// release context //
	SAFE_RELEASE(ScriptContext);
	// return returned value //
	return retrval;
}
// ------------------------------------ //
asIScriptModule* ScriptExecutor::LoadScript(ScriptScript* script){
	// check is it already compiled //
	// do not do it yet, TODO: checksums, file saving/caching

	// create/check module //
	wstring name = script->Source;

	int idnumber = GetModuleNumber(name);
	if(idnumber < 0){
		// no existing module //
		int idtocreate = CreateModule(name, IDFactory::GetID());
		string createname = /*Convert::WstringToString(script->Name)+*/Convert::WstringToString(Convert::IntToWstring(idtocreate));
		asIScriptModule *mod = engine->GetModule(createname.c_str(), asGM_ALWAYS_CREATE);

		if(mod == NULL){
			Logger::Get()->Error(L"ScriptExecutor: LoadScript: Failed to create module with name: "+name, idtocreate, true);
			return NULL;
		}

		string code = "";
		//for(int i = 0; i < script->Instructions.size(); i++){
		//	code += Convert::WstringToString(*script->Instructions[i])+";\n";
		//}
		code = Convert::WstringToString(script->Instructions);
		// add script //
		if(mod->AddScriptSection(Convert::WstringToString(script->Name).c_str(), code.c_str(), code.size()) < 0){
			Logger::Get()->Error(L"ScriptExecutor: LoadScript: Failed to add code to module "+name, idtocreate, false);
			Logger::Get()->Info(L"Code: \n"+Convert::StringToWstring(code), true);
			return NULL;
		}

		if(mod->Build() < 0){
			Logger::Get()->Error(L"ScriptExecutor: LoadScript: Failed to compile module: "+Convert::StringToWstring(createname), idtocreate, true);
			return NULL;
		}
		return mod;
	}
	string createname = /*Convert::WstringToString(script->Name)+*/Convert::WstringToString(Convert::ToWstring(idnumber));
	// get module //
	asIScriptModule *mod = engine->GetModule(createname.c_str());
	if(NULL == mod){
		Logger::Get()->Error(L"ScriptExecutor: LoadScript: Failed to retrieve module with name: "+Convert::StringToWstring(createname), idnumber, true);
		return NULL;
	}

	return mod;
}
int ScriptExecutor::GetModuleNumber(wstring name, int id){
	for(unsigned int i = 0; i < Modules.size(); i++){
		if(name != L""){
			if(Modules[i]->Name != name)
				continue;
		}
		if(id != -1){
			if(Modules[i]->ID != id)
				continue;
		}
		// correct found //
		return Modules[i]->ModuleID;
	}
	return -1;
}
int ScriptExecutor::CreateModule(wstring name, int id){
	Modules.push_back(new ScriptModule(name, id));
	return Modules.back()->ModuleID;
}
// ------------------------------------ //
//int ScriptExecutor::SearchScript(wstring name){
//	for(int i = 0; i < RunningScripts.size(); i++){
//		if(RunningScripts[i]->Name == name)
//			return i;
//	}
//	return -1;
//}

ScriptNamedArguement* ScriptExecutor::GetVariable(wstring& name){
	for(unsigned int i = 0; i < Parameters.size(); i++){
		if(Parameters[i]->Name == name)
			return Parameters[i];

	}
	for(unsigned int i = 0; i < ScriptsValues->Vars.size(); i++){
		if(ScriptsValues->Vars[i]->Name == name)
			return ScriptsValues->Vars[i];

	}
	for(unsigned int i = 0; i < Globals.size(); i++){
		for(unsigned int a = 0; a < Globals[i]->Vars.size(); a++){
			if(Globals[i]->Vars[a]->Name == name){
				return Globals[i]->Vars[a];
			}
		}
	}
	return NULL;
}


