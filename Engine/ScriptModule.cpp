#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_SCRIPTMODULE
#include "ScriptModule.h"
#endif
#include "ScriptInterface.h"
#include <boost\assign\list_of.hpp>
#include "WstringIterator.h"
using namespace Leviathan;
// ------------------------------------ //


ScriptModule::ScriptModule(asIScriptEngine* engine, const wstring &name, int id, const string &source) : FuncParameterInfos(), 
	ScriptBuilder(new CScriptBuilder()), Source(source), ID(id), Name(name), ScriptState(SCRIPTBUILDSTATE_EMPTY), 
	ModuleName(source+"_;"+Convert::ToString<int>(LatestAssigned))
{
	LatestAssigned++;
	
	// module will always be started //
	ScriptBuilder->StartNewModule(engine, ModuleName.c_str());

	ListenerDataBuilt = false;
}

ScriptModule::~ScriptModule(){
	// we'll need to destroy the module from the engine //
	ScriptInterface::Get()->GetExecutor()->GetASEngine()->DiscardModule(ModuleName.c_str());
	// and then delete the builder //
	SAFE_DELETE(ScriptBuilder);
	SAFE_DELETE_VECTOR(FuncParameterInfos);
}

int Leviathan::ScriptModule::LatestAssigned = 0;
map<int, wstring> Leviathan::ScriptModule::EngineTypeIDS;

const map<wstring, int> Leviathan::ScriptModule::ListenerNameType = boost::assign::map_list_of(LISTENERNAME_ONSHOW, LISTENERVALUE_ONSHOW)
	(LISTENERNAME_ONHIDE, LISTENERVALUE_ONHIDE);

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
	asIScriptModule* module = ScriptBuilder->GetModule();

	if(EngineTypeIDS.size() == 0){
		// put basic types //

		int itype = module->GetTypeIdByDecl("int");
		int ftype = module->GetTypeIdByDecl("float");
		int btype = module->GetTypeIdByDecl("bool");
		int stype = module->GetTypeIdByDecl("string");
		int vtype = module->GetTypeIdByDecl("void");
		// add //
		EngineTypeIDS.insert(make_pair(itype, L"int"));
		EngineTypeIDS.insert(make_pair(ftype, L"float"));
		EngineTypeIDS.insert(make_pair(btype, L"bool"));
		EngineTypeIDS.insert(make_pair(stype, L"string"));
		EngineTypeIDS.insert(make_pair(vtype, L"void"));
	}

	// space is already reserved and objects allocated //
	for(UINT i = 0; i < parameterc; i++){
		// get parameter type id //
		int paraid = func->GetParamTypeId(i);

		_FillParameterDataObject(paraid, &newinfo->ParameterTypeIDS[i], &newinfo->ParameterDeclarations[i], &newinfo->MatchingDataBlockTypes[i]);
	}

	// return type //
	int paraid = func->GetReturnTypeId();

	_FillParameterDataObject(paraid, &newinfo->ReturnTypeID, &newinfo->ReturnTypeDeclaration, &newinfo->ReturnMatchingDataBlock);

	// add to vector //
	FuncParameterInfos.push_back(newinfo.release());

	// return generated //
	return FuncParameterInfos.back();
}
// ------------------------------------ //
void Leviathan::ScriptModule::_FillParameterDataObject(int typeofas, asUINT* paramtypeid, wstring* paramdecl, int* datablocktype){
	// set //
	*paramtypeid = typeofas;
	// try to find name //

	auto finder = EngineTypeIDS.find(typeofas);

	if(finder != EngineTypeIDS.end()){

		*paramdecl = EngineTypeIDS[typeofas];
		// check for matching datablock //

		int blocktypeid = Convert::WstringTypeNameCheck(*paramdecl);

		if(blocktypeid < 0){
			// non matching found //
			//// set it to generic pointer //
			//*datablocktype = DATABLOCK_TYPE_VOIDPTR;
			*datablocktype = -1;

		} else {
			// some datablock type supports this //
			*datablocktype = blocktypeid;
		}
	} else {
		// generic pointer type, store name and stuff //

		// get name from engine //
		asIObjectType* typedefinition = GetModule()->GetEngine()->GetObjectTypeById(typeofas);

		*paramdecl = Convert::StringToWstring(typedefinition->GetName());

		// set it to generic pointer //
		*datablocktype = DATABLOCK_TYPE_VOIDPTR;
	}
}
// ------------------------------------ //
DLLEXPORT asIScriptModule* Leviathan::ScriptModule::GetModule(){
	// we need to check build state //
	if(ScriptState == SCRIPTBUILDSTATE_READYTOBUILD){
		// build it //
		int result = ScriptBuilder->BuildModule();
		if(result < 0){
			// failed to build //
			Logger::Get()->Error(L"ScriptModule: GetModule: failed to build unbuilt module, "+GetInfoWstring());
			return NULL;
		}

		ScriptState = SCRIPTBUILDSTATE_BUILT;
	}
	// get module from engine //
	return ScriptInterface::Get()->GetExecutor()->GetASEngine()->GetModule(ModuleName.c_str(), asGM_ONLY_IF_EXISTS);
}
// ------------------------------------ //
DLLEXPORT shared_ptr<ScriptScript> Leviathan::ScriptModule::GetScriptInstance(){

	return shared_ptr<ScriptScript>(new ScriptScript(ID, ScriptInterface::Get()->GetExecutor()->GetModule(ID)));
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::ScriptModule::DoesListenersContainSpecificListener(const wstring &listenername){
	// build info if not built //
	if(!ListenerDataBuilt)
		_BuildListenerList();


	// find from the map //
	auto itr = FoundListenerFunctions.find(listenername);

	if(itr != FoundListenerFunctions.end()){

		// it exists //
		return true;
	}
	// no matching listener //
	return false;
}
// ------------------------------------ //
DLLEXPORT wstring Leviathan::ScriptModule::GetInfoWstring(){
	return L"ScriptModule("+Convert::ToWstring(ID)+L") "+Name+L", from: "+Convert::StringToWstring(Source);
}

DLLEXPORT void Leviathan::ScriptModule::DeleteThisModule(){
	// tell script interface to unload this //
	ScriptInterface::Get()->GetExecutor()->DeleteModule(this);
}
// ------------------------------------ //
void Leviathan::ScriptModule::_BuildListenerList(){

	// we need to find functions with metadata specifying which listener it is //
	asIScriptModule* mod = GetModule();

	asUINT funccount = mod->GetFunctionCount();
	// loop all and check the ones with promising names //
	for(asUINT i = 0; i < funccount; i++){

		asIScriptFunction* tmpfunc = mod->GetFunctionByIndex(i);

		// check does name start with 'O' // 
		//if(tmpfunc->GetName()[0] == 'O'){
		//	// get metadata for this and process //
			_ProcessMetadataForFunc(tmpfunc, mod);
		//}
	}

	//PrintFunctionsInModule();

	// data is now built //
	ListenerDataBuilt = true;
}

void Leviathan::ScriptModule::_ProcessMetadataForFunc(asIScriptFunction* func, asIScriptModule* mod){
	// start of by getting metadata string //
	string meta = ScriptBuilder->GetMetadataStringForFunc(func);

	if(meta.size() < 3){
		// too short for anything //
		return;
	}

	// do all kinds of checks on the metadata //
	if(meta[0] == '@'){
		// some specific special function, check which //

		// we need some iterating here //
		WstringIterator itr(new wstring(Convert::StringToWstring(meta)), true);

		// need to skip first character don't want @ to be in the name //
		itr.MoveToNext();


		// get until assignment //
		unique_ptr<wstring> metaname = itr.GetUntilEqualityAssignment(EQUALITYCHARACTER_TYPE_EQUALITY);

		// check name //
		if(*metaname == L"Listener"){
			// it's a listener function //

			// get string in quotes to find out what it is //
			unique_ptr<wstring> listenername = itr.GetStringInQuotes(QUOTETYPE_BOTH);

			// make iterator to skip spaces //
			itr.SkipWhiteSpace();

			// match listener's name with OnFunction type //
			auto positerator = ListenerNameType.find(*listenername);

			if(positerator != ListenerNameType.end()){
				// found a match, store info //
				FoundListenerFunctions[*listenername] = shared_ptr<ValidListenerData>(new ValidListenerData(func, itr.GetUntilEnd()));

				return;
			}
			// we shouldn't have gotten here, error //
			Logger::Get()->Error(L"ScriptModule: ProcessMetadata: invalid Listener name, "+*listenername);
		}


	}
}



DLLEXPORT void Leviathan::ScriptModule::PrintFunctionsInModule(){
	// list consoles' global variables //
	Logger::Get()->Info(Name+L" instance functions: ");

	// List the user functions in the module
	asIScriptModule* mod = GetModule();

	const asUINT FuncCount = mod->GetFunctionCount();

	for(asUINT n = 0; n < FuncCount; n++ ){
		// get function //
		asIScriptFunction* func = mod->GetFunctionByIndex(n);
		// print the function //
		Logger::Get()->Write(L"> "+Convert::StringToWstring(func->GetName())+L"("+Convert::StringToWstring(func->GetDeclaration())+L")");
	}

	Logger::Get()->Write(L"[END]");
}

Leviathan::ValidListenerData::ValidListenerData(asIScriptFunction* funcptr, unique_ptr<wstring> metadataend) : FuncPtr(funcptr){
	// take ownership of the unique pointer //
	RestOfMeta.swap(metadataend);
	// increase references //
	FuncPtr->AddRef();
}

Leviathan::ValidListenerData::~ValidListenerData(){
	// decrease reference  //
	FuncPtr->Release();
}
