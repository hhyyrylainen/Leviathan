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

// headers that contains bind able functions //
#include "GuiScriptInterface.h"
#include "WstringIterator.h"

ScriptExecutor::ScriptExecutor() : engine(NULL), Modules(), RunningScripts(NULL), Parameters(), Errors(){
	// set default values //
	PrintErrors = true;
	RunType = SCRIPT_EXECUTOR_RUNTYPE_BREAKONERROR;
}
ScriptExecutor::~ScriptExecutor(){
	if(engine)
		// try to release engine //
		Release();
}
int Val_NOVALVAL_FORscriptptr = VAL_NOUPDATE;
// ------------------------------------ //
bool ScriptExecutor::Init(){
	// initialize AngelScript //
	engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	if(engine == NULL){

		Logger::Get()->Error(L"ScriptExecutor: Init: asCreateScriptEngine failed", false);
		return false;
	}
	// set callback to error report function //
	engine->SetMessageCallback(asFUNCTION(ScriptMessageCallback), 0, asCALL_CDECL);

	// register script string type //
	RegisterStdString(engine);

	// register global functions and classes //
	if(engine->RegisterGlobalFunction("void Print(string message, bool save = true)", asFUNCTION(Logger::Print), asCALL_CDECL) < 0){
		// error abort //
		Logger::Get()->Error(L"ScriptExecutor: Init: AngelScript: RegisterGlobalFunction: failed, line: "+Convert::IntToWstring(__LINE__));
		return false;
	}
	// global values //
	if(engine->RegisterGlobalProperty("const int VAL_NOUPDATE", &Val_NOVALVAL_FORscriptptr) < 0){
		// error abort //
		Logger::Get()->Error(L"ScriptExecutor: Init: AngelScript: RegisterGlobalProperty: failed, line: "+Convert::IntToWstring(__LINE__));
		return false;
	}

	// include various headers that bind functions and classes //
#include "GuiScriptBind.h"

	return true;
}
bool ScriptExecutor::Release(){
	// release AngelScript //
	engine->Release();

	while(Modules.size() != 0){
		SAFE_DELETE(Modules[0]);
		Modules.erase(Modules.begin());
	}

	return true;
}
// ------------------------------------ //
DLLEXPORT shared_ptr<VariableBlock> Leviathan::ScriptExecutor::RunScript(ScriptScript* script, vector<shared_ptr<NamedVariableBlock>> parameters, 
	bool printerrors, const wstring &entrance, bool &existsreceiver, bool ErrorIfdoesnt /*= true*/, bool fulldecl /*= false*/, 
	int runtype /*= SCRIPT_EXECUTOR_RUNTYPE_BREAKONERROR*/)
{
	SetScript(script);
	SetParameters(parameters);

	SetBehavior(printerrors, runtype);
	// run //
	shared_ptr<VariableBlock> returnarg = RunSetUp(entrance, fulldecl, ErrorIfdoesnt);

	// clean up//
	Clear();
	return returnarg;
}

shared_ptr<VariableBlock> ScriptExecutor::RunSetUp(const wstring &entrance, bool &existsreceiver, bool fulldecl, bool ErrorIfdoesnt){
	// check data validness //
	if(RunningScripts == NULL){
		Logger::Get()->Error(L"ScriptExecutor: RunSetUp: no script to run!");
		return shared_ptr<VariableBlock>(new VariableBlock(new IntBlock(NULL)));
	}

	// run script //
	// return return value //
	try{
		return RunScript(entrance, existsreceiver, fulldecl, ErrorIfdoesnt);
	}
	catch (ScriptException &e){
		if(PrintErrors){
			Logger::Get()->Error(L"ScriptExecutor:  while executing : "+RunningScripts->Name+L" exception: "+Convert::IntToWstring(e.ErrorCode)+L" "
				+e.Message+L" action "+Convert::IntToWstring(e.ActionValue)+L" from "+e.Source);
		}
		Errors.push_back(new ScriptException(e));
		return shared_ptr<VariableBlock>(new VariableBlock(new IntBlock(80000800)));
	}
	catch (exception &e){
		
		Logger::Get()->Error(L"ScriptExecutor:  exception : "+Convert::StringToWstringNonRef(e.what()), true);

		return shared_ptr<VariableBlock>(new VariableBlock(new IntBlock(80000800)));
	}
	catch (string &e){
		
		Logger::Get()->Error(L"ScriptExecutor:  exception : "+Convert::StringToWstring(e), true);

		return shared_ptr<VariableBlock>(new VariableBlock(new IntBlock(80000800)));
	}
	catch (int e){
		
		Logger::Get()->Error(L"ScriptExecutor:  exception : "+Convert::ToWstring(e), true);

		return shared_ptr<VariableBlock>(new VariableBlock(new IntBlock(80000800)));
	}
	catch (...){
		
		Logger::Get()->Error(L"ScriptExecutor:  exception : UNKOWN EXCEPTION, MIGHT BE FATAL ERROR!", true);

		return shared_ptr<VariableBlock>(new VariableBlock(new IntBlock(80000800)));
	}
}
// ------------------------------------ //
void ScriptExecutor::SetScript(ScriptScript* script){
	RunningScripts = script;
}
void ScriptExecutor::SetBehavior(bool printerrors, int runtype){
	PrintErrors = printerrors;
	RunType = runtype;
}
DLLEXPORT void Leviathan::ScriptExecutor::SetParameters(vector<shared_ptr<NamedVariableBlock>> parameters){
	Parameters = parameters;
}

void ScriptExecutor::Clear(){

	RunningScripts = NULL;
	Parameters.clear();

	while(Errors.size() != 0){
		Logger::Get()->Error(L"ScriptExecutor:  exception: "+Convert::IntToWstring(Errors[0]->ErrorCode)+L" "+Errors[0]->Message+L" action "
			+Convert::IntToWstring(Errors[0]->ActionValue)+L" from "+Errors[0]->Source, true);
		SAFE_DELETE(Errors[0]);
		Errors.erase(Errors.begin());
	}
}
// ------------------------------------ //
shared_ptr<VariableBlock> ScriptExecutor::RunScript(const wstring &start, bool &existsreceiver, bool fulldecl, bool ErrorIfdoesnt){
	// check validness //
	if(RunningScripts->Instructions.size() < 1){
		Logger::Get()->Error(L"ScriptExecutor: RunScript No instructions inside script name: "+RunningScripts->Name, true);
		return shared_ptr<VariableBlock>(new VariableBlock(new IntBlock(80000800)));
	}

	// load script //
	ScriptModule* scrptmodule = NULL;
	asIScriptModule* Module = LoadScript(RunningScripts, &scrptmodule);
	if(Module == NULL){
		// report error and exit //
		Logger::Get()->Error(L"ScriptExecutor: RunScript: Failed to compile script "+RunningScripts->Name, true);
		return shared_ptr<VariableBlock>(new VariableBlock(new IntBlock(80000800)));
	}

	// run script //
	asIScriptContext* ScriptContext = engine->CreateContext();
	if(ScriptContext == NULL){
		Logger::Get()->Error(L"ScriptExecutor: RunScript: Failed to create context for script: "+RunningScripts->Name, true);
		SAFE_RELEASE(ScriptContext);
		return shared_ptr<VariableBlock>(new VariableBlock(new IntBlock(80000800)));
	}

	// run scripts //
	asIScriptFunction *func = NULL;
	string namey = Convert::WstringToString(start);
	if(!fulldecl){
		func = Module->GetFunctionByName(namey.c_str());
	} else {
		func = Module->GetFunctionByDecl(namey.c_str());
	}
	if(func == NULL){
		if(ErrorIfdoesnt){
			Logger::Get()->Error(L"ScriptExecutor: RunScript: Could not find starting function: "+start+L" in: "+RunningScripts->Name, true);
			SAFE_RELEASE(ScriptContext);
			// set exists state //
			existsreceiver = false;

			return shared_ptr<VariableBlock>(new VariableBlock(new IntBlock(80000800)));
		} else {
			// set exists state //
			existsreceiver = false;
			return shared_ptr<VariableBlock>(new VariableBlock(new IntBlock(80000800)));
		}
	}
	// set exists state //
	existsreceiver = true;

	if(ScriptContext->Prepare(func) < 0){
		Logger::Get()->Error(L"ScriptExecutor: RunScript: prepare context failed func: "+start+L" in:"+RunningScripts->Name, true);
		SAFE_RELEASE(ScriptContext);
		return shared_ptr<VariableBlock>(new VariableBlock(new IntBlock(80000800)));
	}

	// pass arguments // // figure out if arguments match function declaration //

	FunctionParameterInfo* paraminfo = scrptmodule->GetParamInfoForFunction(func);

	int parameterc = paraminfo->ParameterTypeIDS.size();

	// start passing parameters //
	for(int i = 0; i < parameterc; i++){
		if(i >= (int)Parameters.size()) // no more parameters //
			break;
		// check do they match //
		bool l_error = false;

		// check for matching types //
		if(paraminfo->MatchingDataBlockTypes[i] != Parameters[i]->GetBlock()->Type){
			// damn //
			// check if they are compatible //
			l_error = true;
		}
		if(l_error){
			Logger::Get()->Error(L"ScriptExecutor: RunScript: pass parameters failed in func: "+start+L" named: "+RunningScripts->Name+L" param number: "+Convert::IntToWstring(i), i, true);
			return shared_ptr<VariableBlock>(new VariableBlock(new IntBlock(80000800)));
		}
		// pass //
		switch(paraminfo->MatchingDataBlockTypes[i]){
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
			asIScriptFunction* exceptionfunc = ScriptContext->GetExceptionFunction();

			Logger::Get()->Error(L"[SCRIPT] [EXCEPTION] from function: "+Convert::ToWstring(func->GetDeclaration())+L" module: "
				+Convert::ToWstring(exceptionfunc->GetModuleName())+L" sect: "+Convert::ToWstring(exceptionfunc->GetScriptSectionName()), false);
			throw ScriptException(retcode, Convert::StringToWstringNonRef(ScriptContext->GetExceptionString()), RunningScripts->Source+
				L" line: "+Convert::ToWstring(ScriptContext->GetExceptionLineNumber()));
		}
		SAFE_RELEASE(ScriptContext);
		return shared_ptr<VariableBlock>(new VariableBlock(new IntBlock(80000800)));
	}
	// successfully executed, try to fetch return value //
	shared_ptr<VariableBlock> retrval;

	if(paraminfo->ReturnTypeID != 0){
		// return type isn't void //
		switch(paraminfo->ReturnMatchingDataBlock){
		case DATABLOCK_TYPE_INT:
			{
				retrval = shared_ptr<VariableBlock>(new VariableBlock(new IntBlock(ScriptContext->GetReturnDWord())));
			}
			break;
		case DATABLOCK_TYPE_FLOAT:
			{
				retrval = shared_ptr<VariableBlock>(new VariableBlock(new FloatBlock(ScriptContext->GetReturnFloat())));
			}
			break;
		case DATABLOCK_TYPE_BOOL:
			{
				retrval = shared_ptr<VariableBlock>(new VariableBlock(new BoolBlock(ScriptContext->GetReturnByte() != 0)));
			}
			break;
		default:
			{
				retrval = shared_ptr<VariableBlock>(new VariableBlock(new WstringBlock(L"Return type not supported")));
				Logger::Get()->Info(L"[SCRIPT] return type not supported"+Convert::StringToWstring(paraminfo->ReturnTypeDeclaration));
			}
		}
	}

	// release context //
	SAFE_RELEASE(ScriptContext);
	// return returned value //
	return retrval;
}
// ------------------------------------ //
asIScriptModule* Leviathan::ScriptExecutor::LoadScript(ScriptScript* script, ScriptModule** fetchmodule){
	// check is it already compiled //
	// do not do it yet, TODO: checksums, file saving/caching

	// create/check module //
	wstring name = script->Source;

	(*fetchmodule) = GetModule(name);
	if((*fetchmodule) == NULL){
		// no existing module //
		(*fetchmodule) = CreateModule(name, IDFactory::GetID(), script);

		asIScriptModule* mod = (*fetchmodule)->GetModule(engine);


		if(mod == NULL){
			Logger::Get()->Error(L"ScriptExecutor: LoadScript: Failed to create module from source: "+script->Source);
			return NULL;
		}

		string code = Convert::WstringToString(script->Instructions);
		// add script //
		if(mod->AddScriptSection(Convert::WstringToString(script->Name).c_str(), code.c_str(), code.size()) < 0){
			Logger::Get()->Error(L"ScriptExecutor: LoadScript: Failed to add code to module "+name);
			Logger::Get()->Info(L"Code: \n"+Convert::StringToWstring(code), true);
			return NULL;
		}

		if(mod->Build() < 0){
			Logger::Get()->Error(L"ScriptExecutor: LoadScript: Failed to compile module: "+(*fetchmodule)->Name);
			return NULL;
		}
		return mod;
	}
	// get module //
	return (*fetchmodule)->GetModule(engine);
}

ScriptModule* ScriptExecutor::GetModule(const wstring &name, int id){
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
		return Modules[i];
	}
	return NULL;
}
ScriptModule* ScriptExecutor::CreateModule(const wstring &name, int id, ScriptScript* scrpt){
	Modules.push_back(new ScriptModule(name, id, scrpt->Name));
	return Modules.back();
}
// ---------------- ScriptModule -------------------- //
ScriptModule::~ScriptModule(){
	Module = NULL;
	ModuleName = "-1";
}
ScriptModule::ScriptModule(const wstring &name, int id, const wstring &scriptname) : FuncParameterInfos(){
	ID = id;
	LatestAssigned++;
	ModuleID = LatestAssigned;
	ModuleName = Convert::WstringToString(scriptname)+"_;"+Convert::ToString<int>(ModuleID);

	Name = name;
}

int ScriptModule::LatestAssigned = -1;

// ------------------------------------ //
FunctionParameterInfo* Leviathan::ScriptModule::GetParamInfoForFunction(asIScriptFunction* func){
	// get function id
	int funcid = func->GetId();

	// search for one //
	for(size_t i = 0; i < FuncParameterInfos.size(); i++){
		// check for matching id //
		if(FuncParameterInfos[i]->FunctionID == funcid){
			return FuncParameterInfos[i];
		}
	}

	unsigned int parameterc = func->GetParamCount();

	// generate new //
	unique_ptr<FunctionParameterInfo> newinfo(new FunctionParameterInfo(funcid, parameterc));

	// fill it with data //



	if(EngineTypeIDS.size() == 0){
		// put basic types //

		int itype = Module->GetTypeIdByDecl("int");
		int ftype = Module->GetTypeIdByDecl("float");
		int btype = Module->GetTypeIdByDecl("bool");
		int stype = Module->GetTypeIdByDecl("string");
		int vtype = Module->GetTypeIdByDecl("void");
		// add //
		EngineTypeIDS.insert(make_pair(itype, "int"));
		EngineTypeIDS.insert(make_pair(ftype, "float"));
		EngineTypeIDS.insert(make_pair(btype, "bool"));
		EngineTypeIDS.insert(make_pair(stype, "string"));
		EngineTypeIDS.insert(make_pair(vtype, "void"));
	}

	// space is already reserved and objects allocated //
	for(UINT i = 0; i < parameterc; i++){
		// get parameter type id //
		int paraid = func->GetParamTypeId(i);

		FillData(paraid, &newinfo->ParameterTypeIDS[i], &newinfo->ParameterDeclarations[i], &newinfo->MatchingDataBlockTypes[i]);
	}

	// return type //
	int paraid = func->GetReturnTypeId();

	FillData(paraid, &newinfo->ReturnTypeID, &newinfo->ReturnTypeDeclaration, &newinfo->ReturnMatchingDataBlock);


	// add to vector //
	FuncParameterInfos.push_back(newinfo.release());

	// return generated //
	return FuncParameterInfos.back();
}

asIScriptModule* Leviathan::ScriptModule::GetModule(asIScriptEngine* engine){
	Module = engine->GetModule(ModuleName.c_str(), asGM_CREATE_IF_NOT_EXISTS);
	return Module;
}

void Leviathan::ScriptModule::FillData(int typeofas, asUINT* paramtypeid, string* paramdecl, int* datablocktype){
	// set //
	*paramtypeid = typeofas;
	// try to find name //

	map<int, string>::iterator finder = EngineTypeIDS.find(typeofas);

	if(finder != EngineTypeIDS.end()){

		*paramdecl = EngineTypeIDS[typeofas];
		// check for matching datablock //

		int blocktypeid = Convert::WstringTypeNameCheck(Convert::StringToWstring(*paramdecl));

		if(blocktypeid < 0){
			// non matching found //
			*datablocktype = -1;

		} else {
			// some datablock type supports this //
			*datablocktype = blocktypeid;
		}
	}
}

map<int, string> Leviathan::ScriptModule::EngineTypeIDS;
