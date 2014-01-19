#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_SCRIPT_EXECUTOR
#include "ScriptExecutor.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
#include "Script/AngelScriptCommon.h"
#include "Utility/Iterators/WstringIterator.h"

#include <add_on/scriptstdstring/scriptstdstring.h>
#include <add_on/scriptarray/scriptarray.h>
#include <add_on/scriptmath/scriptmathcomplex.h>
#include <add_on/scriptmath/scriptmath.h>
#include <add_on/scriptarray/scriptarray.h>
#include <add_on/scriptstdstring/scriptstdstring.h>

// headers that contains bind able functions //
#include "GUI/GuiScriptInterface.h"

// include various headers that bind functions and classes //
#include "GUI/GuiScriptBind.h"
#include "CommonEngineBind.h"
#include <add_on/scripthelper/scripthelper.h>
#include "add_on/scriptdictionary/scriptdictionary.h"
#include "Application/Application.h"

ScriptExecutor::ScriptExecutor() : engine(NULL), AllocatedScriptModules(){
	instance = this;
}
ScriptExecutor::~ScriptExecutor(){
	instance = NULL;

	if(engine)
		// try to release engine //
		Release();
}

DLLEXPORT ScriptExecutor* Leviathan::ScriptExecutor::Get(){
	return instance;
}


ScriptExecutor* Leviathan::ScriptExecutor::instance = NULL;
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
	RegisterScriptArray(engine, true);
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
	//if(engine->RegisterGlobalProperty("const int VAL_NOUPDATE", ) < 0){
	//	// error abort //
	//	Logger::Get()->Error(L"ScriptExecutor: Init: AngelScript: RegisterGlobalProperty: failed, line: "+Convert::IntToWstring(__LINE__));
	//	return false;
	//}

	// use various binding functions //

	// binding Event DataStore DataBlock and others //
	if(!BindEngineCommonScriptIterface(engine)){
		// failed //
		Logger::Get()->Error(L"ScriptExecutor: Init: AngelScript: register Engine object things failed");
		return false;
	}

	// binding TextLabel and other objects to be elements //
	if(!BindGUIObjects(engine)){
		// failed //
		Logger::Get()->Error(L"ScriptExecutor: Init: AngelScript: register GUI object things failed");
		return false;
	}

	// bind application specific //
	Engine::GetEngine()->GetOwningApplication()->InitLoadCustomScriptTypes(engine);

	return true;
}
void ScriptExecutor::Release(){
	// release/delete all modules //
	AllocatedScriptModules.clear();

	// release AngelScript //
	SAFE_RELEASE(engine);

	// release these to stop VLD complains //
	EngineTypeIDS.clear();
	EngineTypeIDSInverted.clear();
}
// ------------------------------------ //
DLLEXPORT shared_ptr<VariableBlock> Leviathan::ScriptExecutor::RunSetUp(ScriptScript* scriptobject, ScriptRunningSetup* parameters){
	// get ScriptModule for script //
	ScriptModule* scrptmodule = scriptobject->GetModule();
	if(!scrptmodule){
		// report error and exit //
		Logger::Get()->Error(L"ScriptExecutor: RunSetUp: trying to run empty module");
		return shared_ptr<VariableBlock>(new VariableBlock(-1));
	}

	// load script //
	asIScriptModule* Module = scrptmodule->GetModule();
	if(!Module){
		// report error and exit //
		Logger::Get()->Error(L"ScriptExecutor: RunSetUp: cannot run invalid script module: "+scrptmodule->GetInfoWstring(), true);
		return shared_ptr<VariableBlock>(new VariableBlock(-1));
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

		return shared_ptr<VariableBlock>(new VariableBlock(-1));
	}

	// create running context for the function //
	asIScriptContext* ScriptContext = engine->CreateContext();
	if(!ScriptContext){
		Logger::Get()->Error(L"ScriptExecutor: RunScript: Failed to create context ", true);
		SAFE_RELEASE(ScriptContext);
		return shared_ptr<VariableBlock>(new VariableBlock(-1));
	}

	// set exists state //
	parameters->ScriptExisted = true;

	if(ScriptContext->Prepare(func) < 0){
		Logger::Get()->Error(L"ScriptExecutor: RunScript: prepare context failed func: "+Convert::StringToWstring(parameters->Entryfunction)+
			L" in: "+scrptmodule->GetInfoWstring(), true);
		SAFE_RELEASE(ScriptContext);
		return shared_ptr<VariableBlock>(new VariableBlock(-1));
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
			const asIScriptFunction* exceptionfunc = ScriptContext->GetExceptionFunction();

			Logger::Get()->Error(L"[SCRIPT] [EXCEPTION] "+Convert::StringToWstring(ScriptContext->GetExceptionString())+L", function: "
				+Convert::ToWstring(func->GetDeclaration())+L"\n\t in "+Convert::ToWstring(exceptionfunc->GetScriptSectionName())+L" ("+
				Convert::ToWstring(ScriptContext->GetExceptionLineNumber()+L") "+scrptmodule->GetInfoWstring()));


			PrintAdditionalExcept(ScriptContext);
		}

		ScriptContext->Release();
		return shared_ptr<VariableBlock>(new VariableBlock(-1));
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
	ScriptContext->Release();
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

DLLEXPORT bool Leviathan::ScriptExecutor::DeleteModuleIfNoExternalReferences(int ID){
	// Find based on the id //
	// find module based on pointer and remove //
	for(size_t i = 0; i < AllocatedScriptModules.size(); i++){
		if(AllocatedScriptModules[i]->GetID() == ID){
			// Check reference count //
			if(AllocatedScriptModules[i].use_count() != 1){
				// Other references exist //
				return false;
			}

			// remove //
			AllocatedScriptModules.erase(AllocatedScriptModules.begin()+i);
			return true;
		}
	}
	// Nothing found //
	return false;
}
// ------------------------------------ //
void Leviathan::ScriptExecutor::PrintAdditionalExcept(asIScriptContext *ctx){
	// Print callstack as additional information //
	Logger::Get()->Write(L"// ------------------ CallStack ------------------ //\n");
	// Loop the stack starting from the frame below the current function (actually might be nice to print the top frame too) //
	for(UINT n = 0; n < ctx->GetCallstackSize(); n++){
		// Get the function object //
		const asIScriptFunction* function = ctx->GetFunction(n);
		// If we function doesn't exist this frame is used internally by the script engine //
		if(function){
			// Check function type //
			if(function->GetFuncType() == asFUNC_SCRIPT){
				// Print info about the script function //
				Logger::Get()->Write(L"\t> "+Convert::StringToWstring(function->GetScriptSectionName())+L" ("+Convert::ToWstring(ctx->GetLineNumber(n))
					+L"): "+Convert::StringToWstring(function->GetDeclaration())+L"\n");
			} else {
				// Info about the application functions //
				// The context is being reused by the application for a nested call
				Logger::Get()->Write(L"\t> {...Application...}: "+Convert::StringToWstring(function->GetDeclaration())+L"\n");
			}
		} else {
			// The context is being reused by the script engine for a nested call
			Logger::Get()->Write(L"\t> {...Script internal...}\n");
		}
	}
}
// ------------------------------------ //
DLLEXPORT void Leviathan::ScriptExecutor::ScanAngelScriptTypes(){
	if(EngineTypeIDS.size() == 0){
		// put basic types //
		EngineTypeIDS.insert(make_pair(engine->GetTypeIdByDecl("int"), L"int"));
		EngineTypeIDS.insert(make_pair(engine->GetTypeIdByDecl("float"), L"float"));
		EngineTypeIDS.insert(make_pair(engine->GetTypeIdByDecl("bool"), L"bool"));
		EngineTypeIDS.insert(make_pair(engine->GetTypeIdByDecl("string"), L"string"));
		EngineTypeIDS.insert(make_pair(engine->GetTypeIdByDecl("void"), L"void"));
	}
	// call some callbacks //
	RegisterGUIScriptTypeNames(engine, EngineTypeIDS);
	RegisterEngineScriptTypes(engine, EngineTypeIDS);

	Engine::GetEngine()->GetOwningApplication()->RegisterCustomScriptTypes(engine, EngineTypeIDS);


	// invert the got list, since it should be final //
	for(auto iter = EngineTypeIDS.begin(); iter != EngineTypeIDS.end(); ++iter){

		EngineTypeIDSInverted.insert(make_pair(iter->second, iter->first));
	}
}

std::map<wstring, int> Leviathan::ScriptExecutor::EngineTypeIDSInverted;

std::map<int, wstring> Leviathan::ScriptExecutor::EngineTypeIDS;

