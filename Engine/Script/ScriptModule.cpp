#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_SCRIPTMODULE
#include "ScriptModule.h"
#endif
#include "ScriptInterface.h"
#include <boost/assign/list_of.hpp>
#include "Iterators/StringIterator.h"
#include "FileSystem.h"
#include "Common/StringOperations.h"
using namespace Leviathan;
// ------------------------------------ //


ScriptModule::ScriptModule(asIScriptEngine* engine, const wstring &name, int id, const string &source) : FuncParameterInfos(), 
	ScriptBuilder(new CScriptBuilder()), Source(source), ID(id), Name(name), ScriptState(SCRIPTBUILDSTATE_EMPTY), 
	ModuleName(source+"_;"+Convert::ToString<int>(LatestAssigned))
{
	LatestAssigned++;
	
	// module will always be started //
	ScriptBuilder->StartNewModule(engine, ModuleName.c_str());

	// setup include resolver //
	ScriptBuilder->SetIncludeCallback(ScriptModuleIncludeCallback, this);

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

const map<wstring, int> Leviathan::ScriptModule::ListenerNameType = boost::assign::map_list_of(LISTENERNAME_ONSHOW, LISTENERVALUE_ONSHOW)
	(LISTENERNAME_ONHIDE, LISTENERVALUE_ONHIDE) (LISTENERNAME_ONLISTENUPDATE, LISTENERVALUE_ONSHOW) (LISTENERNAME_ONCLICK, LISTENERVALUE_ONCLICK)
	(LISTENERNAME_ONVALUECHANGE, LISTENERVALUE_ONVALUECHANGE) (LISTENERNAME_ONINIT, LISTENERVALUE_ONINIT) 
	(LISTENERNAME_ONRELEASE, LISTENERVALUE_ONRELEASE) (LISTENERNAME_ONSUBMIT, LISTENERVALUE_ONSUBMIT) (LISTENERNAME_ONTICK, LISTENERVALUE_ONTICK)
	(LISTENERNAME_ONCLOSECLICKED, LISTENERVALUE_ONCLOSECLICKED);

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

	auto finder = ScriptExecutor::EngineTypeIDS.find(typeofas);

	if(finder != ScriptExecutor::EngineTypeIDS.end()){

		*paramdecl = ScriptExecutor::EngineTypeIDS[typeofas];
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
		// Add the source files before building //

		for(size_t i = 0; i < ScriptSourceSegments.size(); i++){
			if(ScriptBuilder->AddSectionFromMemory(ScriptSourceSegments[i]->SourceFile.c_str(), ScriptSourceSegments[i]->SourceCode->c_str(),
				ScriptSourceSegments[i]->StartLine) < 0)
			{
				Logger::Get()->Error(L"ScriptModule: GetModule: failed to build unbuilt module (adding source files failed), "+GetInfoWstring());
				ScriptState = SCRIPTBUILDSTATE_FAILED;
				return NULL;
			}
		}


		// Build it //
		int result = ScriptBuilder->BuildModule();
		Logger::Get()->Save();
		if(result < 0){
			// failed to build //
			Logger::Get()->Error(L"ScriptModule: GetModule: failed to build unbuilt module, "+GetInfoWstring());
			ScriptState = SCRIPTBUILDSTATE_FAILED;
			return NULL;
		}

		ScriptState = SCRIPTBUILDSTATE_BUILT;

	} else if(ScriptState == SCRIPTBUILDSTATE_FAILED){

		return NULL;
	}

	// Get module from the engine //
	return ScriptInterface::Get()->GetExecutor()->GetASEngine()->GetModule(ModuleName.c_str(), asGM_ONLY_IF_EXISTS);
}
// ------------------------------------ //
DLLEXPORT shared_ptr<ScriptScript> Leviathan::ScriptModule::GetScriptInstance(){

	return shared_ptr<ScriptScript>(new ScriptScript(ID, ScriptInterface::Get()->GetExecutor()->GetModule(ID)));
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::ScriptModule::DoesListenersContainSpecificListener(const wstring &listenername, const wstring* generictype /*= NULL*/){

	// find from the map //
	auto itr = _GetIteratorOfListener(listenername, generictype);

	if(itr != FoundListenerFunctions.end()){

		// it exists //
		return true;
	}
	// no matching listener //
	return false;
}

DLLEXPORT void Leviathan::ScriptModule::GetListOfListeners(std::vector<shared_ptr<ValidListenerData>> &receiver){
	// build info if not built //
	if(!ListenerDataBuilt)
		_BuildListenerList();
	// reserve space to be more efficient //
	receiver.reserve(FoundListenerFunctions.size());
	

	for(auto iter = FoundListenerFunctions.begin(); iter != FoundListenerFunctions.end(); ++iter) {
		receiver.push_back(iter->second);
	}
}

DLLEXPORT string Leviathan::ScriptModule::GetListeningFunctionName(const wstring &listenername, const wstring* generictype /*= NULL*/){
	// call search function and check if it found anything //
	auto itr = _GetIteratorOfListener(listenername, generictype);
	

	if(itr != FoundListenerFunctions.end()){
		// get name from pointer //
		return string(itr->second->FuncPtr->GetName());
	}
	// nothing found //
	return "";
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

	if(!mod){
		// module failed to build //
		return;
	}

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
		StringIterator itr(Convert::StringToWstring(meta));

		// need to skip first character don't want @ to be in the name //
		itr.MoveToNext();


		// get until assignment //
		auto metaname = itr.GetUntilEqualityAssignment<wstring>(EQUALITYCHARACTER_TYPE_EQUALITY);

		// check name //
		if(*metaname == L"Listener"){
			// it's a listener function //

			// get string in quotes to find out what it is //
			auto listenername = itr.GetStringInQuotes<wstring>(QUOTETYPE_BOTH);

			wstring localname = *listenername;

			// if it is generic listener we need to get it's type //
			if(*listenername == L"Generic"){

				auto generictype = itr.GetStringInQuotes<wstring>(QUOTETYPE_BOTH);

				if(generictype->size() == 0){

					Logger::Get()->Warning(L"ScriptModule: ProcessMetadata: Generic listener has no type defined (expected declaration like \""
						L"[@Listener=\"Generic\", @Type=\"ScoreUpdated\"]\"");
					return;
				}

				// mash together a name //
				wstring mangledname = L"Generic:"+*generictype+L";";
				auto restofmeta = itr.GetUntilEnd<wstring>();
				FoundListenerFunctions[mangledname] = shared_ptr<ValidListenerData>(new ValidListenerData(func, listenername.release(), 
					restofmeta.release(), generictype.release()));

				return;
			}


			// make iterator to skip spaces //
			itr.SkipWhiteSpace();

			// match listener's name with OnFunction type //
			auto positerator = ListenerNameType.find(*listenername);

			if(positerator != ListenerNameType.end()){
				// found a match, store info //
				auto restofmeta = itr.GetUntilEnd<wstring>();
				FoundListenerFunctions[localname] = shared_ptr<ValidListenerData>(new ValidListenerData(func, listenername.release(), 
					restofmeta.release()));

				return;
			}
			// we shouldn't have gotten here, error //
			Logger::Get()->Error(L"ScriptModule: ProcessMetadata: invalid Listener name, "+*listenername);
		}


	}
}


std::map<wstring, shared_ptr<ValidListenerData>>::iterator Leviathan::ScriptModule::_GetIteratorOfListener(const wstring &listenername, 
	const wstring* generictype /*= NULL*/)
{
	// build info if not built //
	if(!ListenerDataBuilt)
		_BuildListenerList();

	// find from the map //
	std::map<wstring, shared_ptr<ValidListenerData>>::iterator itr = FoundListenerFunctions.end();

	// different implementations for generic finding, because the name isn't usable as is //
	if(!generictype){
		// default find is fine for known types //
		itr = FoundListenerFunctions.find(listenername);
	} else {
		// we need to find the right one by comparing strings inside the objects //
		for(auto iter = FoundListenerFunctions.begin(); iter != FoundListenerFunctions.end(); ++iter){
			// strings are sorted alphabetically so we can skip until "Generic" //
			if(iter->first.at(0) != L'G')
				continue;
			// check for matching generic name //
			if(*iter->second->GenericTypeName == *generictype){
				// found right one, copy iterator and return //
				itr = iter;
				break;
			}
		}

	}
	// return whatever we might have found at this point //
	return itr;
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

DLLEXPORT int Leviathan::ScriptModule::ScriptModuleIncludeCallback(const char* include, const char* from, CScriptBuilder* builder, void* userParam){
	// by default we need to try to add include to from path and try to open it //
	string file(include);
	string infile(from);

	ScriptModule* module = reinterpret_cast<ScriptModule*>(userParam);

	// if it is prefixed with ".\" or "./" then just look for the file with it's relative path //
	if(file.find(".\\") == 0 || file.find("./") == 0){

		return builder->AddSectionFromFile(file.c_str());
	}

	size_t lastpathseparator = infile.find_last_of('/');
	if(lastpathseparator != string::npos){
		string justpath = infile.substr(0, lastpathseparator+1); 

		string completefile = justpath += file;

		if(FileSystem::FileExists(completefile)){
			// completed search //
			return builder->AddSectionFromFile(completefile.c_str());
		} else {
			
			goto trytofindinscriptfolderincludecallback;
		}
	} else {
trytofindinscriptfolderincludecallback:

		wstring wfile = Convert::StringToWstring(file);

		// try to find in script folder //
		wstring extension = StringOperations::GetExtensionWstring(wfile);

		wstring name = StringOperations::RemoveExtensionWstring(wfile, true);

		// search //
		wstring finalpath = FileSystem::Get()->SearchForFile(FILEGROUP_SCRIPT, name, extension, false);

		if(finalpath.size() > 0){

			return builder->AddSectionFromFile(Convert::WstringToString(finalpath).c_str());
		} else {

			Logger::Get()->Error(L"ScriptModule: IncludeCallback: couldn't resolve include (even with full search), file: "+wfile
				+L" in "+module->GetInfoWstring());
		}
	}

	// if we got here the file couldn't be found //
	return -1;
}
// ------------------------------------ //
DLLEXPORT size_t Leviathan::ScriptModule::GetScriptSegmentCount() const{
	return ScriptSourceSegments.size();
}

DLLEXPORT shared_ptr<ScriptSourceFileData> Leviathan::ScriptModule::GetScriptSegment(size_t index) const{
	if(index >= ScriptSourceSegments.size())
		return NULL;

	return ScriptSourceSegments[index];
}

DLLEXPORT bool Leviathan::ScriptModule::AddScriptSegment(shared_ptr<ScriptSourceFileData> data){
	// Check is it already there //
	for(size_t i = 0; i < ScriptSourceSegments.size(); i++){

		if(ScriptSourceSegments[i]->SourceFile == data->SourceFile && (abs(ScriptSourceSegments[i]->StartLine-data->StartLine) <= 2)){

			return false;
		}
	}

	ScriptSourceSegments.push_back(data);
	return true;
}

DLLEXPORT bool Leviathan::ScriptModule::AddScriptSegmentFromFile(const string &file){
	// Check is it already there //
	for(size_t i = 0; i < ScriptSourceSegments.size(); i++){

		if(ScriptSourceSegments[i]->SourceFile == file){

			return false;
		}
	}

	// Load the source code from the file //
	string scriptdata;
	FileSystem::ReadFileEntirely(file, scriptdata);

	ScriptSourceSegments.push_back(shared_ptr<ScriptSourceFileData>(new ScriptSourceFileData(file, 1, scriptdata)));
	return true;
}

DLLEXPORT const string& Leviathan::ScriptModule::GetModuleName() const{
	return ModuleName;
}
// ------------------ ValidListenerData ------------------ //
Leviathan::ValidListenerData::ValidListenerData(asIScriptFunction* funcptr, wstring* name, wstring* metadataend) 
	: FuncPtr(funcptr), ListenerName(name), RestOfMeta(metadataend)
{
	// increase references //
	FuncPtr->AddRef();
}

Leviathan::ValidListenerData::ValidListenerData(asIScriptFunction* funcptr, wstring* name, wstring* metadataend, wstring* generictypename) 
	: FuncPtr(funcptr), ListenerName(name), GenericTypeName(generictypename), RestOfMeta(metadataend)
{
	// increase references //
	FuncPtr->AddRef();
}

Leviathan::ValidListenerData::~ValidListenerData(){
	// decrease reference  //
	FuncPtr->Release();
}
// ------------------ ScriptSourceFileData ------------------ //
Leviathan::ScriptSourceFileData::ScriptSourceFileData(const string &file, int line, const string &code) : SourceFile(file), StartLine(line), 
	SourceCode(new string(code))
{

}
