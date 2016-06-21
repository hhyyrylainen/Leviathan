// ------------------------------------ //
#include "ScriptModule.h"

#include "ScriptExecutor.h"
#include <boost/assign/list_of.hpp>
#include "Iterators/StringIterator.h"
#include "FileSystem.h"
#include "Common/StringOperations.h"
#include "Handlers/ResourceRefreshHandler.h"
#include "add_on/serializer/serializer.h"
#include "Events/CallableObject.h"
using namespace Leviathan;
using namespace std;
// ------------------------------------ //
ScriptModule::ScriptModule(asIScriptEngine* engine, const std::string &name, int id,
    const string &source) :
    Name(name), Source(source), ID(id), ScriptBuilder(new CScriptBuilder())
{
	{
		Lock lock(ModuleBuildingMutex);
        ModuleName = string(source+"_;"+Convert::ToString<int>(LatestAssigned));
		LatestAssigned++;
	}

	// module will always be started //
	ScriptBuilder->StartNewModule(engine, ModuleName.c_str());

	// setup include resolver //
	ScriptBuilder->SetIncludeCallback(ScriptModuleIncludeCallback, this);

	ListenerDataBuilt = false;
}

ScriptModule::~ScriptModule(){

}

DLLEXPORT void Leviathan::ScriptModule::Release(){
	GUARD_LOCK();

	// Leave from the bridge //
	if(ArgsBridge)
		ArgsBridge->LeaveModule();


#ifdef SCRIPTMODULE_LISTENFORFILECHANGES

	_StopFileMonitoring(guard);

#endif // SCRIPTMODULE_LISTENFORFILECHANGES

	// We'll need to destroy the module from the engine //
	ASModule = NULL;
	ScriptExecutor::Get()->GetASEngine()->DiscardModule(ModuleName.c_str());

	// And then delete the builder //
	SAFE_DELETE(ScriptBuilder);
	SAFE_DELETE_VECTOR(FuncParameterInfos);
	ListenerDataBuilt = false;
}

int Leviathan::ScriptModule::LatestAssigned = 0;

const map<std::string, int> Leviathan::ScriptModule::ListenerNameType = boost::assign::map_list_of
    (LISTENERNAME_ONSHOW, LISTENERVALUE_ONSHOW)(LISTENERNAME_ONHIDE, LISTENERVALUE_ONHIDE)
    (LISTENERNAME_ONLISTENUPDATE, LISTENERVALUE_ONSHOW) (LISTENERNAME_ONCLICK, LISTENERVALUE_ONCLICK)
    (LISTENERNAME_ONVALUECHANGE, LISTENERVALUE_ONVALUECHANGE) (LISTENERNAME_ONINIT, LISTENERVALUE_ONINIT)
    (LISTENERNAME_ONRELEASE, LISTENERVALUE_ONRELEASE) (LISTENERNAME_ONSUBMIT, LISTENERVALUE_ONSUBMIT)
    (LISTENERNAME_ONTICK, LISTENERVALUE_ONTICK) (LISTENERNAME_ONCLOSECLICKED, LISTENERVALUE_ONCLOSECLICKED)
    (LISTENERNAME_LISTSELECTIONACCEPTED, LISTENERVALUE_LISTSELECTIONACCEPTED);

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

    // This verifies that the module is built //
	ScriptBuilder->GetModule();

	// space is already reserved and objects allocated //
	for(unsigned int i = 0; i < parameterc; i++){
		// get parameter type id //
		int paraid;
		func->GetParam(i, &paraid);

		_FillParameterDataObject(paraid, &newinfo->ParameterTypeIDS[i],
            &newinfo->ParameterDeclarations[i],
            &newinfo->MatchingDataBlockTypes[i]);
	}

	// return type //
	int paraid = func->GetReturnTypeId();

	_FillParameterDataObject(paraid, &newinfo->ReturnTypeID, &newinfo->ReturnTypeDeclaration,
        &newinfo->ReturnMatchingDataBlock);

	// add to vector //
	FuncParameterInfos.push_back(newinfo.release());

	// return generated //
	return FuncParameterInfos.back();
}
// ------------------------------------ //
void Leviathan::ScriptModule::_FillParameterDataObject(int typeofas, asUINT* paramtypeid,
    std::string* paramdecl, int* datablocktype)
{
	// set //
	*paramtypeid = typeofas;
	// try to find name //

	auto finder = ScriptExecutor::EngineTypeIDS.find(typeofas);

	if(finder != ScriptExecutor::EngineTypeIDS.end()){

		*paramdecl = ScriptExecutor::EngineTypeIDS[typeofas];
		// check for matching datablock //

		int blocktypeid = Convert::StringTypeNameCheck(*paramdecl);

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
		asITypeInfo* typedefinition = GetModule()->GetEngine()->GetTypeInfoById(typeofas);

		*paramdecl = typedefinition->GetName();

		// set it to generic pointer //
		*datablocktype = DATABLOCK_TYPE_VOIDPTR;
	}
}
// ------------------------------------ //
DLLEXPORT asIScriptModule* Leviathan::ScriptModule::GetModule(Lock &guard){

	// we need to check build state //
	if(ScriptState == SCRIPTBUILDSTATE_READYTOBUILD){

		_BuildTheModule(guard);

#ifdef SCRIPTMODULE_LISTENFORFILECHANGES

		_StartMonitoringFiles(guard);

#endif // SCRIPTMODULE_LISTENFORFILECHANGES

		if(ScriptState != SCRIPTBUILDSTATE_BUILT){

			return NULL;
		}

	} else if(ScriptState == SCRIPTBUILDSTATE_FAILED || ScriptState == SCRIPTBUILDSTATE_DISCARDED){

		return NULL;
	}

	// Return the saved pointer or fetch the pointer //
	if(!ASModule){
        
		// Get module from the engine //
		ASModule = ScriptExecutor::Get()->GetASEngine()->GetModule(ModuleName.c_str(),
            asGM_ONLY_IF_EXISTS);

		if(!ASModule){

			// The module is invalid //
			Logger::Get()->Error("ScriptModule: GetModule: module is no longer anywhere to be found in the AS engine");
			ScriptState = SCRIPTBUILDSTATE_DISCARDED;
			return NULL;
		}
	}

	return ASModule;
}
// ------------------------------------ //
DLLEXPORT std::shared_ptr<ScriptScript> Leviathan::ScriptModule::GetScriptInstance(){

	return std::shared_ptr<ScriptScript>(new ScriptScript(ID, ScriptExecutor::Get()->GetModule(ID)));
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::ScriptModule::DoesListenersContainSpecificListener(const std::string &listenername,
    const std::string* generictype /*= NULL*/)
{
	GUARD_LOCK();
	// find from the map //
	auto itr = _GetIteratorOfListener(guard, listenername, generictype);

	if(itr != FoundListenerFunctions.end()){

		// it exists //
		return true;
	}
	// no matching listener //
	return false;
}

DLLEXPORT void Leviathan::ScriptModule::GetListOfListeners(std::vector<shared_ptr<ValidListenerData>> &receiver){
	GUARD_LOCK();
	// build info if not built //
	if(!ListenerDataBuilt)
		_BuildListenerList(guard);
	// reserve space to be more efficient //
	receiver.reserve(FoundListenerFunctions.size());
	

	for(auto iter = FoundListenerFunctions.begin(); iter != FoundListenerFunctions.end(); ++iter) {
		receiver.push_back(iter->second);
	}
}

DLLEXPORT string Leviathan::ScriptModule::GetListeningFunctionName(const std::string &listenername,
    const std::string* generictype /*= NULL*/)
{
	GUARD_LOCK();
    
	// Call search function and check if it found anything //
	auto itr = _GetIteratorOfListener(guard, listenername, generictype);
	

	if(itr != FoundListenerFunctions.end()){
		// Get name from pointer //
		return string(itr->second->FuncPtr->GetName());
	}
    
	// Nothing found //
	return "";
}
// ------------------------------------ //
DLLEXPORT std::string Leviathan::ScriptModule::GetInfoString(){
    
	return "ScriptModule("+Convert::ToString(ID)+") "+Name+", from: "+Source;
}

DLLEXPORT void Leviathan::ScriptModule::DeleteThisModule(){
    
	// Tell script interface to unload this //
	ScriptExecutor::Get()->DeleteModule(this);
}
// ------------------------------------ //
void Leviathan::ScriptModule::_BuildListenerList(Lock &guard){

	VerifyLock(guard);

	// We need to find functions with metadata specifying which listener it is //
	asIScriptModule* mod = GetModule(guard);

	if(!mod){
		// Module failed to build //
		return;
	}

	asUINT funccount = mod->GetFunctionCount();
	// Loop all and check the ones with promising names //
	for(asUINT i = 0; i < funccount; i++){

		asIScriptFunction* tmpfunc = mod->GetFunctionByIndex(i);


		// Get metadata for this and process it //
		_ProcessMetadataForFunc(tmpfunc, mod);
	}

	// Data is now built //
	ListenerDataBuilt = true;
}

void Leviathan::ScriptModule::_ProcessMetadataForFunc(asIScriptFunction* func,
    asIScriptModule* mod)
{
    
	// Start of by getting metadata string //
	string meta = ScriptBuilder->GetMetadataStringForFunc(func);

	if(meta.size() < 3){
		// Too short for anything //
		return;
	}

	// Do all kinds of checks on the metadata //
	if(meta[0] == '@'){
		// Some specific special function, check which //

		// We need some iterating here //
		StringIterator itr(meta);

		// need to skip first character don't want @ to be in the name //
		itr.MoveToNext();

		// get until assignment //
		auto metaname = itr.GetUntilEqualityAssignment<string>(EQUALITYCHARACTER_TYPE_EQUALITY);

		// check name //
		if(*metaname == "Listener"){
			// it's a listener function //

			// get string in quotes to find out what it is //
			auto listenername = itr.GetStringInQuotes<std::string>(QUOTETYPE_BOTH);

			std::string localname = *listenername;

			// if it is generic listener we need to get it's type //
			if(*listenername == "Generic"){

				auto generictype = itr.GetStringInQuotes<std::string>(QUOTETYPE_BOTH);

				if(generictype->size() == 0){

					Logger::Get()->Warning("ScriptModule: ProcessMetadata: Generic listener has "
                        "no type defined (expected declaration like \""
						"[@Listener=\"Generic\", @Type=\"ScoreUpdated\"]\"");
					return;
				}

				// mash together a name //
				std::string mangledname = "Generic:"+*generictype+";";
				auto restofmeta = itr.GetUntilEnd<std::string>();
                
				FoundListenerFunctions[mangledname] = std::shared_ptr<ValidListenerData>(
                    new ValidListenerData(func, listenername.release(), 
					restofmeta.release(), generictype.release()));

				return;
			}


			// make iterator to skip spaces //
			itr.SkipWhiteSpace();

			// match listener's name with OnFunction type //
			auto positerator = ListenerNameType.find(*listenername);

			if(positerator != ListenerNameType.end()){
				// found a match, store info //
				auto restofmeta = itr.GetUntilEnd<std::string>();
				FoundListenerFunctions[localname] = std::shared_ptr<ValidListenerData>(
                    new ValidListenerData(func, listenername.release(), 
					restofmeta.release()));

				return;
			}
			// we shouldn't have gotten here, error //
			Logger::Get()->Error("ScriptModule: ProcessMetadata: invalid Listener name, "+*listenername);
		}


	}
}


std::map<std::string, std::shared_ptr<ValidListenerData>>::iterator
    Leviathan::ScriptModule::_GetIteratorOfListener(Lock &guard, const std::string &listenername,
        const std::string* generictype /*= NULL*/)
{
	// build info if not built //
	if(!ListenerDataBuilt)
		_BuildListenerList(guard);

	// find from the map //
	auto itr = FoundListenerFunctions.end();

	// different implementations for generic finding, because the name isn't usable as is //
	if(!generictype){
		// default find is fine for known types //
		itr = FoundListenerFunctions.find(listenername);
        
	} else {
		// we need to find the right one by comparing strings inside the objects //
		for(auto iter = FoundListenerFunctions.begin(); iter != FoundListenerFunctions.end();
            ++iter)
        {
			// strings are sorted alphabetically so we can skip until "Generic" //
			if(iter->first.at(0) != 'G')
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

	GUARD_LOCK();
	// list consoles' global variables //
	Logger::Get()->Info(Name+" instance functions: ");

	// List the user functions in the module
	asIScriptModule* mod = GetModule();

	const asUINT FuncCount = mod->GetFunctionCount();

	for(asUINT n = 0; n < FuncCount; n++ ){
		// get function //
		asIScriptFunction* func = mod->GetFunctionByIndex(n);
		// print the function //
		Logger::Get()->Write(string("> ")+func->GetName()+"("+func->GetDeclaration()+")");
	}

	Logger::Get()->Write("[END]");
}

DLLEXPORT int Leviathan::ScriptModule::ScriptModuleIncludeCallback(const char* include,
    const char* from, CScriptBuilder* builder, void* userParam)
{
	// by default we need to try to add include to from path and try to open it //
	string file(include);
	string infile(from);

	ScriptModule* module = reinterpret_cast<ScriptModule*>(userParam);
    // The module has to be locked during this call

	// if it is prefixed with ".\" or "./" then just look for the file with it's relative path //
	if(file.find(".\\") == 0 || file.find("./") == 0){

#ifdef SCRIPTMODULE_LISTENFORFILECHANGES

		module->_AddFileToMonitorIfNotAlready(file);
#endif // SCRIPTMODULE_LISTENFORFILECHANGES

        const string beginstripped = file.substr(2, file.size()-2);
        
		return builder->AddSectionFromFile(file.c_str());
	}

	size_t lastpathseparator = infile.find_last_of('/');
	if(lastpathseparator != string::npos){
		string justpath = infile.substr(0, lastpathseparator+1); 

		string completefile = justpath += file;

		if(FileSystem::FileExists(completefile)){
			// completed search //
#ifdef SCRIPTMODULE_LISTENFORFILECHANGES

			module->_AddFileToMonitorIfNotAlready(completefile);
#endif // SCRIPTMODULE_LISTENFORFILECHANGES

            if(completefile.find("./") == 0)
                completefile = completefile.substr(2, completefile.size()-2);

			return builder->AddSectionFromFile(completefile.c_str());
		} else {
			
			goto trytofindinscriptfolderincludecallback;
		}
	} else {
trytofindinscriptfolderincludecallback:

		// try to find in script folder //
		std::string extension = StringOperations::GetExtensionString(file);

		std::string name = StringOperations::RemoveExtensionString(file, true);

		// search //
		std::string finalpath = FileSystem::Get()->SearchForFile(FILEGROUP_SCRIPT, name,
            extension, false);

		if(finalpath.size() > 0){

#ifdef SCRIPTMODULE_LISTENFORFILECHANGES

			module->_AddFileToMonitorIfNotAlready(finalpath);
#endif // SCRIPTMODULE_LISTENFORFILECHANGES

            if(finalpath.find("./") == 0)
                finalpath = finalpath.substr(2, finalpath.size()-2);

			return builder->AddSectionFromFile(finalpath.c_str());
		} else {

			Logger::Get()->Error("ScriptModule: IncludeCallback: couldn't resolve include "
                "(even with full search), file: "+file+" in "+module->GetInfoString());
		}
	}

	// if we got here the file couldn't be found //
	return -1;
}
// ------------------------------------ //
DLLEXPORT size_t Leviathan::ScriptModule::GetScriptSegmentCount() const{
	return ScriptSourceSegments.size();
}

DLLEXPORT std::shared_ptr<ScriptSourceFileData> Leviathan::ScriptModule::GetScriptSegment(
    size_t index) const
{
	if(index >= ScriptSourceSegments.size())
		return NULL;

	return ScriptSourceSegments[index];
}

DLLEXPORT bool Leviathan::ScriptModule::AddScriptSegment(shared_ptr<ScriptSourceFileData> data){
	GUARD_LOCK();
	// Check is it already there //
	for(size_t i = 0; i < ScriptSourceSegments.size(); i++){

		if(ScriptSourceSegments[i]->SourceFile == data->SourceFile &&
            (abs(ScriptSourceSegments[i]->StartLine-data->StartLine) <= 2))
        {

			return false;
		}
	}

	ScriptSourceSegments.push_back(data);

    // Needs to be built next //
    ScriptState = SCRIPTBUILDSTATE_READYTOBUILD;
	return true;
}

DLLEXPORT bool Leviathan::ScriptModule::AddScriptSegmentFromFile(const string &file){
	GUARD_LOCK();
	// Check is it already there //
	for(size_t i = 0; i < ScriptSourceSegments.size(); i++){

		if(ScriptSourceSegments[i]->SourceFile == file){

			return false;
		}
	}

	// Load the source code from the file //
	string scriptdata;
	FileSystem::ReadFileEntirely(file, scriptdata);

	ScriptSourceSegments.push_back(shared_ptr<ScriptSourceFileData>(
            new ScriptSourceFileData(file, 1, scriptdata)));
    
    // Needs to be built next //
    ScriptState = SCRIPTBUILDSTATE_READYTOBUILD;
	return true;
}

DLLEXPORT const string& Leviathan::ScriptModule::GetModuleName() const{
	return ModuleName;
}
// ------------------------------------ //
DLLEXPORT void Leviathan::ScriptModule::SetAsInvalid(){
	GUARD_LOCK();

	ScriptState = SCRIPTBUILDSTATE_DISCARDED;
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::ScriptModule::ReLoadModuleCode(){

	GUARD_LOCK();

	// The module must be still valid //
	if(ScriptState == SCRIPTBUILDSTATE_DISCARDED ||
        (!GetModule() && ScriptState != SCRIPTBUILDSTATE_FAILED))
		return false;


	Logger::Get()->Info("Reloading "+GetInfoString());

	if(ArgsBridge){

		// Do a OnRelease event call //
		const std::string& listenername =
            CallableObject::GetListenerNameFromType(EVENT_TYPE_RELEASE);

		// check does the script contain right listeners //
		if(DoesListenersContainSpecificListener(listenername)){

			// Get the parameters //
			auto sargs = ArgsBridge->GetProvider()->GetParametersForRelease();
			sargs->SetEntrypoint(GetListeningFunctionName(listenername));

			// Run the script //
			ScriptExecutor::Get()->RunSetUp(this, sargs.get());

			Logger::Get()->Info("ScriptModule: ran auto release");
		}
	}


	// Store the old values //
	//CSerializer backup;
	//backup.Store(ASModule);

	// Discard the old module //
	ASModule = NULL;
	if(GetModule())
        ScriptExecutor::Get()->GetASEngine()->DiscardModule(ModuleName.c_str());

	// The builder must be created again //
	SAFE_DELETE(ScriptBuilder);
	SAFE_DELETE_VECTOR(FuncParameterInfos);

	// Setup the new builder //
	ScriptBuilder = new CScriptBuilder();


	asIScriptEngine* engine = ScriptExecutor::Get()->GetASEngine();

	// Add 'R' to the module name to mark reload //
	ModuleName += "R";

	ScriptBuilder->StartNewModule(engine, ModuleName.c_str());
	ScriptBuilder->SetIncludeCallback(ScriptModuleIncludeCallback, this);
	ListenerDataBuilt = false;

	// Build the module //
	_BuildTheModule(guard);

	if(ScriptState != SCRIPTBUILDSTATE_BUILT || !GetModule()){

		// Failed to build //
		return false;
	}

	// Restore the data //
	//backup.Restore(GetModule());

	//Logger::Get()->Info("Successfully restored "+GetInfoStd::String());

	if(ArgsBridge){

		// Do a OnRelease event call //
		const std::string& listenername = CallableObject::GetListenerNameFromType(EVENT_TYPE_INIT);

		// check does the script contain right listeners //
		if(DoesListenersContainSpecificListener(listenername)){

			// Get the parameters //
			auto sargs = ArgsBridge->GetProvider()->GetParametersForInit();
			sargs->SetEntrypoint(GetListeningFunctionName(listenername));

			// Run the script //
			ScriptExecutor::Get()->RunSetUp(this, sargs.get());

			Logger::Get()->Info("ScriptModule: ran auto init after restore");
		}
	}


	// Succeeded //
	return true;
}

#ifdef SCRIPTMODULE_LISTENFORFILECHANGES

void Leviathan::ScriptModule::_StartMonitoringFiles(Lock& guard){

	// First add all the known source files //
	auto end = ScriptSourceSegments.end();
	for(auto iter = ScriptSourceSegments.begin(); iter != end; ++iter){


		_AddFileToMonitorIfNotAlready((*iter)->SourceFile);
	}

	// The others should already have been included //
	// Create listeners and we are good to go //
	auto tmphandler = ResourceRefreshHandler::Get();

	if(!tmphandler){

		return;
	}


	auto end2 = AlreadyMonitoredFiles.end();
	for(auto iter = AlreadyMonitoredFiles.begin(); iter != end2; ++iter){

		// Skip if already added //
		if((*iter)->Added)
			continue;

		std::vector<const std::string*> targetfiles;

		// Add our file //
		targetfiles.push_back((*iter)->File.get());

		// Set this as added to avoid duplicates //
		(*iter)->Added = true;

		// Find all files that are in the same folder //
		std::string basepath = StringOperations::GetPathString(*(*iter)->File);

		for(auto iter2 = iter+1; iter2 != end2; ++iter2){

			if(basepath == StringOperations::GetPathString(*(*iter2)->File) && !(*iter2)->Added){
				
				// This is in the same folder and thus can be monitored by the same listener //
				targetfiles.push_back((*iter2)->File.get());

				// This, too, is now added //
				(*iter2)->Added = true;
			}
		}

		// Start monitoring for them //
		int listenerid;
		tmphandler->ListenForFileChanges(targetfiles, std::bind(&ScriptModule::_FileChanged,
                this, placeholders::_1, placeholders::_2), listenerid);

		FileListeners.push_back(listenerid);
	}
}

void Leviathan::ScriptModule::_StopFileMonitoring(Lock &guard){

	auto tmphandler = ResourceRefreshHandler::Get();

	if(tmphandler){

		// Stop listening for all the files at once //
		auto end = FileListeners.end();
		for(auto iter = FileListeners.begin(); iter != end; ++iter){

			tmphandler->StopListeningForFileChanges(*iter);
		}

		FileListeners.clear();


	}

	AlreadyMonitoredFiles.clear();


	// Everything should be cleared now //

}

void Leviathan::ScriptModule::_AddFileToMonitorIfNotAlready(const string &file){

	// Look for a matching string //
	auto end = AlreadyMonitoredFiles.end();
	for(auto iter = AlreadyMonitoredFiles.begin(); iter != end; ++iter){

		if(*(*iter)->File == file)
			return;
	}


	// Add it as it isn't there yet //
	AlreadyMonitoredFiles.push_back(make_unique<AutomonitoredFile>(file));
}

void Leviathan::ScriptModule::_FileChanged(const std::string &file,
    ResourceFolderListener &caller)
{

	GUARD_LOCK();

	// This ignores multiple messages //
	if(!caller.IsAFileStillUpdated())
		return;

	// Mark everything as not updated //
	auto tmphandler = ResourceRefreshHandler::Get();

	if(tmphandler){

		tmphandler->MarkListenersAsNotUpdated(FileListeners);
	}

	// Reload the module //
	if(!ReLoadModuleCode()){

		Logger::Get()->Error("ScriptModule: FileChanged: failed to reload the module, "+
            GetInfoString());
	}
}

#endif // SCRIPTMODULE_LISTENFORFILECHANGES
// ------------------------------------ //
void Leviathan::ScriptModule::_BuildTheModule(Lock &guard){
	// Add the source files before building //
	for(size_t i = 0; i < ScriptSourceSegments.size(); i++){
		if(ScriptBuilder->AddSectionFromMemory(ScriptSourceSegments[i]->SourceFile.c_str(), 
                ScriptSourceSegments[i]->SourceCode->c_str(), 0,
                ScriptSourceSegments[i]->StartLine-1) < 0)
		{
			Logger::Get()->Error("ScriptModule: GetModule: failed to build unbuilt module "
                "(adding source files failed), "+GetInfoString());
            
			ScriptState = SCRIPTBUILDSTATE_FAILED;
			return;
		}
	}

	// Build it //
	int result;
	{
		// Only one script can be built at a time so a lock is required //
		Lock lock(ModuleBuildingMutex);
		result = ScriptBuilder->BuildModule();
	}

	if(result < 0){
		// failed to build //
		Logger::Get()->Error("ScriptModule: GetModule: failed to build unbuilt module, "+
            GetInfoString());
        
		ScriptState = SCRIPTBUILDSTATE_FAILED;
		return;
	}

	ASModule = ScriptBuilder->GetModule();
	ScriptState = SCRIPTBUILDSTATE_BUILT;
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::ScriptModule::OnAddedToBridge(shared_ptr<ScriptArgumentsProviderBridge> bridge){

	assert(this == bridge->GetModule() && "OnAddedToBridge it's not ours!");

	if(ArgsBridge){

		return false;
	}
	
	ArgsBridge = bridge;
	return true;
}
// ------------------------------------ //
Mutex Leviathan::ScriptModule::ModuleBuildingMutex;

// ------------------ ValidListenerData ------------------ //
Leviathan::ValidListenerData::ValidListenerData(asIScriptFunction* funcptr, std::string* name,
    std::string* metadataend) 
	: FuncPtr(funcptr), ListenerName(name), RestOfMeta(metadataend)
{
	// increase references //
	FuncPtr->AddRef();
}

Leviathan::ValidListenerData::ValidListenerData(asIScriptFunction* funcptr, std::string* name,
    std::string* metadataend, std::string* generictypename) 
	: FuncPtr(funcptr), ListenerName(name), 
      RestOfMeta(metadataend), GenericTypeName(generictypename)
{
	// increase references //
	FuncPtr->AddRef();
}

Leviathan::ValidListenerData::~ValidListenerData(){
	// decrease reference  //
	FuncPtr->Release();
}
// ------------------ ScriptSourceFileData ------------------ //
DLLEXPORT ScriptSourceFileData::ScriptSourceFileData(const string &file, int line,
    const string &code) :
    SourceFile(file), StartLine(line), 
	SourceCode(new string(code))
{

}
