#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_SCRIPT_EXECUTOR
#include "ScriptExecutor.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
#include "Script\AngelScriptCommon.h"
#include "Utility\Iterators\WstringIterator.h"

#include <add_on\scriptstdstring\scriptstdstring.h>
#include <add_on\scriptarray\scriptarray.h>
#include <add_on\scriptmath\scriptmathcomplex.h>
#include <add_on\scriptmath\scriptmath.h>
#include <add_on\scriptarray\scriptarray.h>
#include <add_on\scriptstdstring\scriptstdstring.h>

// headers that contains bind able functions //
#include "GUI\GuiScriptInterface.h"

// include various headers that bind functions and classes //
#include "GUI\GuiScriptBind.h"
#include <add_on\scripthelper\scripthelper.h>
#include "add_on\scriptdictionary\scriptdictionary.h"

ScriptExecutor::ScriptExecutor() : engine(NULL), AllocatedScriptModules(){
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

	// math functions //
	RegisterScriptMath(engine);
	RegisterScriptMathComplex(engine);

	// register script string type //
	RegisterStdString(engine);
	RegisterScriptArray(engine, false);
	// register other script extensions //
	RegisterStdStringUtils(engine);

	// register dictionary object //
	RegisterScriptDictionary_Native(engine);


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

	// use various binding functions //
	if(!BindGUIScriptCommon(engine)){
		// failed //
		Logger::Get()->Error(L"ScriptExecutor: Init: AngelScript: register GUI things failed");
		return false;
	}

	// binding TextLabel and other objects to be elements //
	if(!BindGUIObjects(engine)){
		// failed //
		Logger::Get()->Error(L"ScriptExecutor: Init: AngelScript: register GUI object things failed");
		return false;
	}

	return true;
}
void ScriptExecutor::Release(){
	// release/delete all modules //
	AllocatedScriptModules.clear();

	// release AngelScript //
	SAFE_RELEASE(engine);

	// release these to stop VLD complains //
	ScriptModule::EngineTypeIDS.clear();
}
// ------------------------------------ //
DLLEXPORT shared_ptr<VariableBlock> Leviathan::ScriptExecutor::RunSetUp(ScriptScript* scriptobject, ScriptRunningSetup* parameters){
	// get ScriptModule for script //
	ScriptModule* scrptmodule = scriptobject->GetModule();
	if(!scrptmodule){
		// report error and exit //
		Logger::Get()->Error(L"ScriptExecutor: RunSetUp: trying to run empty module");
		return shared_ptr<VariableBlock>(new VariableBlock(80000800));
	}

	// load script //
	asIScriptModule* Module = scrptmodule->GetModule();
	if(!Module){
		// report error and exit //
		Logger::Get()->Error(L"ScriptExecutor: RunSetUp: cannot run invalid script module: "+scrptmodule->GetInfoWstring(), true);
		return shared_ptr<VariableBlock>(new VariableBlock(80000800));
	}

	// get function pointer to start function //
	asIScriptFunction *func = NULL;
	// get entry function from the module //
	if(!parameters->FullDeclaration){
		func = Module->GetFunctionByName(parameters->Entryfunction.c_str());
	} else {
		func = Module->GetFunctionByDecl(parameters->Entryfunction.c_str());
	}
	if(func == NULL){
		// set exists state //
		parameters->ScriptExisted = false;
		// check should we print an error //
		if(parameters->PrintErrors && parameters->ErrorOnNonExistingFunction){
			Logger::Get()->Error(L"ScriptExecutor: RunScript: Could not find starting function: "+Convert::StringToWstring(parameters->Entryfunction)+
				L" in: "+scrptmodule->GetInfoWstring(), true);
			scrptmodule->PrintFunctionsInModule();
		}

		return shared_ptr<VariableBlock>(new VariableBlock(80000800));
	}

	// create running context for the function //
	asIScriptContext* ScriptContext = engine->CreateContext();
	if(!ScriptContext){
		Logger::Get()->Error(L"ScriptExecutor: RunScript: Failed to create context ", true);
		SAFE_RELEASE(ScriptContext);
		return shared_ptr<VariableBlock>(new VariableBlock(80000800));
	}

	// set exists state //
	parameters->ScriptExisted = true;

	if(ScriptContext->Prepare(func) < 0){
		Logger::Get()->Error(L"ScriptExecutor: RunScript: prepare context failed func: "+Convert::StringToWstring(parameters->Entryfunction)+
			L" in: "+scrptmodule->GetInfoWstring(), true);
		SAFE_RELEASE(ScriptContext);
		return shared_ptr<VariableBlock>(new VariableBlock(80000800));
	}
	// pass arguments //

	// figure out if arguments match function declaration //
	FunctionParameterInfo* paraminfo = scrptmodule->GetParamInfoForFunction(func);

	int parameterc = paraminfo->ParameterTypeIDS.size();

	// start passing parameters //
	for(int i = 0; i < parameterc; i++){
		if(i >= (int)parameters->Parameters.size()) // no more parameters //
			break;
		// try to pass the parameter //
		switch(paraminfo->MatchingDataBlockTypes[i]){
			case DATABLOCK_TYPE_INT:
				{
					int tmpparam = 0;

					if(!parameters->Parameters[i]->ConvertAndAssingToVariable(tmpparam)){

						goto scriptexecutorpassparamsinvalidconversionparam;
					}

					ScriptContext->SetArgDWord(i, tmpparam);
				}
			break;
			case DATABLOCK_TYPE_FLOAT:
				{
					float tmpparam = 0;

					if(!parameters->Parameters[i]->ConvertAndAssingToVariable(tmpparam)){

						goto scriptexecutorpassparamsinvalidconversionparam;
					}

					ScriptContext->SetArgFloat(i, tmpparam);
				}
			break;
			case DATABLOCK_TYPE_BOOL: case DATABLOCK_TYPE_CHAR:
				{
					char tmpparam = 0;

					if(!parameters->Parameters[i]->ConvertAndAssingToVariable(tmpparam)){

						goto scriptexecutorpassparamsinvalidconversionparam;
					}

					ScriptContext->SetArgByte(i, tmpparam);
				}
			break;
			case DATABLOCK_TYPE_WSTRING:
				{
					// save as a string that can be retrieved //
					// not done
					goto scriptexecutorpassparamsinvalidconversionparam;
				}
			break;
			case DATABLOCK_TYPE_VOIDPTR:
				{
					// we need to make sure that script object name matches engine object name //
					if(paraminfo->ParameterDeclarations[i] != parameters->Parameters[i]->GetName()){
						// non matching pointer types //
						Logger::Get()->Error(L"Mismatching ptr types, comparing "+paraminfo->ParameterDeclarations[i]+L" to passed type of "
							+parameters->Parameters[i]->GetName());
					} else {
						// types match, we can pass in the raw pointer //
						void* ptrtostuff = (void*)(*parameters->Parameters[i]);
						ScriptContext->SetArgAddress(i, ptrtostuff);

					}
				}
			break;
			default:
				goto scriptexecutorpassparamsinvalidconversionparam;
		}

		continue;

scriptexecutorpassparamsinvalidconversionparam:


		Logger::Get()->Error(L"ScriptExecutor: RunScript: pass parameters failed func: "+Convert::StringToWstring(parameters->Entryfunction)+
			L" param number: "+Convert::IntToWstring(i)+L" in: "+scrptmodule->GetInfoWstring(), true);
		return shared_ptr<VariableBlock>(new VariableBlock(80000800));
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

			PrintException(ScriptContext, true);

			Logger::Get()->Error(L"[SCRIPT] [EXCEPTION] "+Convert::StringToWstring(ScriptContext->GetExceptionString())+L"from function: "
				+Convert::ToWstring(func->GetDeclaration())+L" "+scrptmodule->GetInfoWstring()
				+L" line: "+Convert::ToWstring(ScriptContext->GetExceptionLineNumber())+L" section: "+
				Convert::ToWstring(exceptionfunc->GetScriptSectionName()));
		}

		SAFE_RELEASE(ScriptContext);
		return shared_ptr<VariableBlock>(new VariableBlock(80000800));
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
		case DATABLOCK_TYPE_CHAR:
			{
				retrval = shared_ptr<VariableBlock>(new VariableBlock(new CharBlock(ScriptContext->GetReturnByte())));
			}
		break;
		default:
			{
				retrval = shared_ptr<VariableBlock>(new VariableBlock(wstring(L"Return type not supported")));
				Logger::Get()->Info(L"[SCRIPT] return type not supported"+paraminfo->ReturnTypeDeclaration);
			}
		}
	}

	// release context //
	SAFE_RELEASE(ScriptContext);
	// return returned value //
	return retrval;
}
// ------------------------------------ //
DLLEXPORT weak_ptr<ScriptModule> Leviathan::ScriptExecutor::GetModule(const int &ID){
	// loop modules and return a ptr to matching id //
	for(size_t i = 0; i < AllocatedScriptModules.size(); i++){
		if(AllocatedScriptModules[i]->GetID() == ID)
			return AllocatedScriptModules[i];
	}
	return shared_ptr<ScriptModule>(NULL);
}

DLLEXPORT weak_ptr<ScriptModule> Leviathan::ScriptExecutor::CreateNewModule(const wstring &name, const string &source, const int &modulesid /*= IDFactory::GetID()*/){
	// create new module to a smart pointer //
	shared_ptr<ScriptModule> tmpptr(new ScriptModule(engine, name, modulesid, source));

	// add to vector and return //
	AllocatedScriptModules.push_back(tmpptr);
	return tmpptr;
}

DLLEXPORT void Leviathan::ScriptExecutor::DeleteModule(ScriptModule* ptrtomatch){
	// find module based on pointer and remove //
	for(size_t i = 0; i < AllocatedScriptModules.size(); i++){
		if(AllocatedScriptModules[i].get() == ptrtomatch){
			// remove //
			AllocatedScriptModules.erase(AllocatedScriptModules.begin()+i);
			return;
		}
	}
}

